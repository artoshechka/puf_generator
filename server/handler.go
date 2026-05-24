package main

import (
	"context"
	"crypto/subtle"
	"encoding/json"
	"errors"
	"log/slog"
	"net/http"
	"strings"
	"sync"
	"time"

	"github.com/jackc/pgx/v5"
	"golang.org/x/time/rate"
)

// Server объединяет слой хранения, конфигурацию и метрики для HTTP-обработчиков.
type Server struct {
	store     *Store
	cfg       Config
	metrics   *Metrics
	limiterMu sync.Mutex
	limiters  map[string]*limiterEntry
}

// limiterEntry — rate.Limiter с отметкой последнего обращения; используется для GC.
type limiterEntry struct {
	limiter  *rate.Limiter
	lastUsed time.Time
}

const (
	maxBodyBytes         = 1 << 20
	maxDeviceIDLen       = 128
	maxFingerprintHexLen = 2048
	verifyRatePerSec     = 2
	verifyBurst          = 5
	limiterIdleTTL       = 10 * time.Minute
	limiterGCInterval    = 5 * time.Minute
)

// routes регистрирует все REST-эндпоинты и возвращает настроенный ServeMux.
func (s *Server) routes() http.Handler {
	mux := http.NewServeMux()
	mux.HandleFunc("POST /devices/{id}/enroll", s.adminAuth(s.handleEnroll))
	mux.HandleFunc("POST /devices/{id}/verify", s.rateLimit(s.handleVerify))
	mux.HandleFunc("GET /devices", s.adminAuth(s.handleList))
	mux.HandleFunc("DELETE /devices/{id}", s.adminAuth(s.handleDelete))
	mux.HandleFunc("GET /metrics", s.adminAuth(s.handleMetrics))
	mux.HandleFunc("GET /health", s.handleHealth)
	return requestLogger(mux)
}

func (s *Server) rateLimit(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		id := r.PathValue("id")
		if limiter := s.limiterFor(id); !limiter.Allow() {
			logger(r.Context()).Warn("rate limit hit", "device_id", id)
			writeError(w, http.StatusTooManyRequests, "rate limit exceeded")
			return
		}
		next(w, r)
	}
}

func (s *Server) limiterFor(id string) *rate.Limiter {
	s.limiterMu.Lock()
	defer s.limiterMu.Unlock()
	if s.limiters == nil {
		s.limiters = make(map[string]*limiterEntry)
	}
	entry, ok := s.limiters[id]
	if !ok {
		entry = &limiterEntry{
			limiter: rate.NewLimiter(rate.Limit(verifyRatePerSec), verifyBurst),
		}
		s.limiters[id] = entry
	}
	entry.lastUsed = time.Now()
	return entry.limiter
}

// gcLimiters периодически удаляет limiter'ы, к которым не обращались дольше limiterIdleTTL.
// Запускается отдельной горутиной из main; останавливается при отмене ctx.
func (s *Server) gcLimiters(ctx context.Context) {
	t := time.NewTicker(limiterGCInterval)
	defer t.Stop()
	for {
		select {
		case <-ctx.Done():
			return
		case now := <-t.C:
			s.limiterMu.Lock()
			removed := 0
			for id, e := range s.limiters {
				if now.Sub(e.lastUsed) > limiterIdleTTL {
					delete(s.limiters, id)
					removed++
				}
			}
			remaining := len(s.limiters)
			s.limiterMu.Unlock()
			if removed > 0 {
				slog.Debug("limiter GC", "removed", removed, "remaining", remaining)
			}
		}
	}
}

func validateDeviceID(id string) error {
	if id == "" {
		return errors.New("device_id must not be empty")
	}
	if len(id) > maxDeviceIDLen {
		return errors.New("device_id is too long")
	}
	return nil
}

// handleEnroll сохраняет эталонный PUF-отпечаток устройства (только для администратора).
// Повторная регистрация заменяет предыдущий отпечаток.
func (s *Server) handleEnroll(w http.ResponseWriter, r *http.Request) {
	log := logger(r.Context())
	id := r.PathValue("id")
	if err := validateDeviceID(id); err != nil {
		log.Warn("enroll bad device_id", "err", err)
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}

	r.Body = http.MaxBytesReader(w, r.Body, maxBodyBytes)

	var body struct {
		Fingerprint string `json:"fingerprint"`
	}
	if err := json.NewDecoder(r.Body).Decode(&body); err != nil {
		log.Warn("enroll bad json", "device_id", id, "err", err)
		writeError(w, http.StatusBadRequest, "invalid JSON")
		return
	}
	if len(body.Fingerprint) > maxFingerprintHexLen {
		log.Warn("enroll fingerprint too long", "device_id", id, "len", len(body.Fingerprint))
		writeError(w, http.StatusBadRequest, "fingerprint is too long")
		return
	}
	fp, err := parseHex(body.Fingerprint, 0)
	if err != nil {
		log.Warn("enroll bad fingerprint", "device_id", id, "err", err)
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}

	if err := s.store.Enroll(r.Context(), id, strings.ToLower(body.Fingerprint)); err != nil {
		log.Error("enroll storage", "device_id", id, "err", err)
		writeError(w, http.StatusInternalServerError, "storage error")
		return
	}

	s.metrics.EnrollTotal.Add(1)
	log.Info("enrolled", "device_id", id, "fingerprint_bytes", len(fp))
	w.WriteHeader(http.StatusCreated)
	writeJSON(r.Context(), w, map[string]string{"device_id": id, "status": "enrolled"})
}

// handleVerify аутентифицирует устройство по PUF-отпечатку.
// Устройство передаёт свежий отпечаток в "Authorization: PUF <hex>".
// Возвращает 200 + JSON при успехе, 401 если HD превышает порог.
func (s *Server) handleVerify(w http.ResponseWriter, r *http.Request) {
	log := logger(r.Context())
	id := r.PathValue("id")
	if err := validateDeviceID(id); err != nil {
		log.Warn("verify bad device_id", "err", err)
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}

	fpHex := extractPUF(r)
	if fpHex == "" {
		log.Warn("verify missing puf header", "device_id", id)
		w.Header().Set("WWW-Authenticate", `PUF realm="puf-server"`)
		writeError(w, http.StatusUnauthorized, "Authorization: PUF <hex-fingerprint> required")
		return
	}
	if len(fpHex) > maxFingerprintHexLen {
		log.Warn("verify fingerprint too long", "device_id", id, "len", len(fpHex))
		writeError(w, http.StatusBadRequest, "fingerprint is too long")
		return
	}

	device, err := s.store.Get(r.Context(), id)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			s.metrics.VerifyNotFound.Add(1)
			log.Warn("verify device not enrolled", "device_id", id)
			writeError(w, http.StatusNotFound, "device not enrolled")
			return
		}
		log.Error("verify get device", "device_id", id, "err", err)
		writeError(w, http.StatusInternalServerError, "storage error")
		return
	}

	reference, err := parseHex(device.FingerprintHex, 0)
	if err != nil {
		log.Error("verify parse reference", "device_id", id, "err", err)
		writeError(w, http.StatusInternalServerError, "storage error")
		return
	}
	candidate, err := parseHex(fpHex, len(reference))
	if err != nil {
		log.Warn("verify parse candidate", "device_id", id, "err", err)
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}

	hd, err := fractionalHD(reference, candidate)
	if err != nil {
		log.Warn("verify hd", "device_id", id, "err", err)
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}
	hdPct := hd * 100.0
	ok := hdPct <= s.cfg.ThresholdPct

	s.metrics.RecordVerify(ok, hdPct)
	if ok {
		log.Debug("verify ok",
			"device_id", id,
			"hd_pct", hdPct,
			"threshold_pct", s.cfg.ThresholdPct,
			"fingerprint_bytes", len(reference),
		)
	} else {
		log.Warn("verify rejected",
			"device_id", id,
			"hd_pct", hdPct,
			"threshold_pct", s.cfg.ThresholdPct,
			"fingerprint_bytes", len(reference),
		)
		w.WriteHeader(http.StatusUnauthorized)
	}
	writeJSON(r.Context(), w, map[string]any{"ok": ok})
}

// handleList возвращает список всех зарегистрированных устройств (только для администратора).
func (s *Server) handleList(w http.ResponseWriter, r *http.Request) {
	log := logger(r.Context())
	devices, err := s.store.List(r.Context())
	if err != nil {
		log.Error("list devices", "err", err)
		writeError(w, http.StatusInternalServerError, "storage error")
		return
	}

	type item struct {
		ID         string `json:"id"`
		EnrolledAt string `json:"enrolled_at"`
	}
	out := make([]item, len(devices))
	for i, d := range devices {
		out[i] = item{ID: d.ID, EnrolledAt: d.EnrolledAt.Format(time.RFC3339)}
	}
	log.Info("listed devices", "count", len(out))
	writeJSON(r.Context(), w, out)
}

// handleDelete удаляет устройство из базы данных (только для администратора).
func (s *Server) handleDelete(w http.ResponseWriter, r *http.Request) {
	log := logger(r.Context())
	id := r.PathValue("id")
	if err := validateDeviceID(id); err != nil {
		log.Warn("delete bad device_id", "err", err)
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}

	found, err := s.store.Delete(r.Context(), id)
	if err != nil {
		log.Error("delete device", "device_id", id, "err", err)
		writeError(w, http.StatusInternalServerError, "storage error")
		return
	}
	if !found {
		log.Info("delete miss", "device_id", id)
		writeError(w, http.StatusNotFound, "device not found")
		return
	}
	log.Info("deleted", "device_id", id)
	w.WriteHeader(http.StatusNoContent)
}

// adminAuth — middleware, требующий "Authorization: Bearer <ADMIN_TOKEN>".
func (s *Server) adminAuth(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		auth := r.Header.Get("Authorization")
		token := strings.TrimPrefix(auth, "Bearer ")
		if subtle.ConstantTimeCompare([]byte(token), []byte(s.cfg.AdminToken)) != 1 {
			logger(r.Context()).Warn("admin auth failed",
				"remote", r.RemoteAddr,
				"path", r.URL.Path,
				"has_header", auth != "",
			)
			writeError(w, http.StatusUnauthorized, "invalid admin token")
			return
		}
		next(w, r)
	}
}

// extractPUF извлекает hex-отпечаток из заголовка "Authorization: PUF <hex>".
// Принимает любой пробельный разделитель между схемой и значением.
func extractPUF(r *http.Request) string {
	auth := r.Header.Get("Authorization")
	const scheme = "PUF"
	if !strings.HasPrefix(auth, scheme) {
		return ""
	}
	rest := auth[len(scheme):]
	trimmed := strings.TrimLeft(rest, " \t")
	if trimmed == rest {
		return "" // отсутствует разделитель — это не схема PUF
	}
	return strings.TrimSpace(trimmed)
}

func writeJSON(ctx context.Context, w http.ResponseWriter, v any) {
	w.Header().Set("Content-Type", "application/json")
	if err := json.NewEncoder(w).Encode(v); err != nil {
		logger(ctx).Error("write json", "err", err)
	}
}

func writeError(w http.ResponseWriter, code int, msg string) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(code)
	if err := json.NewEncoder(w).Encode(map[string]string{"error": msg}); err != nil {
		slog.Error("write error response", "err", err, "status", code)
	}
}
