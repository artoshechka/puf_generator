package main

import (
	"encoding/json"
	"errors"
	"log/slog"
	"net/http"
	"strings"
	"time"

	"github.com/jackc/pgx/v5"
)

// Server объединяет слой хранения, конфигурацию и метрики для HTTP-обработчиков.
type Server struct {
	store   *Store
	cfg     Config
	metrics Metrics
}

// routes регистрирует все REST-эндпоинты и возвращает настроенный ServeMux.
func (s *Server) routes() http.Handler {
	mux := http.NewServeMux()
	mux.HandleFunc("POST /devices/{id}/enroll", s.adminAuth(s.handleEnroll))
	mux.HandleFunc("POST /devices/{id}/verify", s.handleVerify)
	mux.HandleFunc("GET /devices", s.adminAuth(s.handleList))
	mux.HandleFunc("DELETE /devices/{id}", s.adminAuth(s.handleDelete))
	mux.HandleFunc("GET /metrics", s.handleMetrics)
	mux.HandleFunc("GET /health", s.handleHealth)
	return requestLogger(mux)
}

// handleEnroll сохраняет эталонный PUF-отпечаток устройства (только для администратора).
// Повторная регистрация заменяет предыдущий отпечаток.
func (s *Server) handleEnroll(w http.ResponseWriter, r *http.Request) {
	id := r.PathValue("id")

	var body struct {
		Fingerprint string `json:"fingerprint"`
	}
	if err := json.NewDecoder(r.Body).Decode(&body); err != nil {
		writeError(w, http.StatusBadRequest, "invalid JSON")
		return
	}
	if _, err := parseHex(body.Fingerprint); err != nil {
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}

	if err := s.store.Enroll(r.Context(), id, strings.ToLower(body.Fingerprint)); err != nil {
		slog.Error("enroll", "device_id", id, "err", err)
		writeError(w, http.StatusInternalServerError, "storage error")
		return
	}

	s.metrics.EnrollTotal.Add(1)
	w.WriteHeader(http.StatusCreated)
	writeJSON(w, map[string]string{"device_id": id, "status": "enrolled"})
}

// handleVerify аутентифицирует устройство по PUF-отпечатку.
// Устройство передаёт свежий отпечаток в "Authorization: PUF <hex>".
// Возвращает 200 + JSON при успехе, 401 если HD превышает порог.
func (s *Server) handleVerify(w http.ResponseWriter, r *http.Request) {
	id := r.PathValue("id")

	fpHex := extractPUF(r)
	if fpHex == "" {
		w.Header().Set("WWW-Authenticate", `PUF realm="puf-server"`)
		writeError(w, http.StatusUnauthorized, "Authorization: PUF <hex-fingerprint> required")
		return
	}

	candidate, err := parseHex(fpHex)
	if err != nil {
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}

	device, err := s.store.Get(r.Context(), id)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			s.metrics.VerifyNotFound.Add(1)
			writeError(w, http.StatusNotFound, "device not enrolled")
			return
		}
		slog.Error("get device", "device_id", id, "err", err)
		writeError(w, http.StatusInternalServerError, "storage error")
		return
	}

	reference, _ := parseHex(device.FingerprintHex)
	hdPct := fractionalHD(reference, candidate) * 100.0
	ok := hdPct <= s.cfg.ThresholdPct

	s.metrics.RecordVerify(ok, hdPct)
	slog.Info("verify",
		"device_id", id,
		"ok", ok,
		"hamming_pct", hdPct,
		"threshold_pct", s.cfg.ThresholdPct,
	)

	if !ok {
		w.WriteHeader(http.StatusUnauthorized)
	}
	writeJSON(w, map[string]any{
		"ok":            ok,
		"hamming_pct":   hdPct,
		"threshold_pct": s.cfg.ThresholdPct,
	})
}

// handleList возвращает список всех зарегистрированных устройств (только для администратора).
func (s *Server) handleList(w http.ResponseWriter, r *http.Request) {
	devices, err := s.store.List(r.Context())
	if err != nil {
		slog.Error("list devices", "err", err)
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
	writeJSON(w, out)
}

// handleDelete удаляет устройство из базы данных (только для администратора).
func (s *Server) handleDelete(w http.ResponseWriter, r *http.Request) {
	id := r.PathValue("id")

	found, err := s.store.Delete(r.Context(), id)
	if err != nil {
		slog.Error("delete device", "device_id", id, "err", err)
		writeError(w, http.StatusInternalServerError, "storage error")
		return
	}
	if !found {
		writeError(w, http.StatusNotFound, "device not found")
		return
	}
	w.WriteHeader(http.StatusNoContent)
}

// adminAuth — middleware, требующий "Authorization: Bearer <ADMIN_TOKEN>".
// Если ADMIN_TOKEN не задан, эндпоинт открыт (режим разработки).
func (s *Server) adminAuth(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if s.cfg.AdminToken != "" {
			token := strings.TrimPrefix(r.Header.Get("Authorization"), "Bearer ")
			if token != s.cfg.AdminToken {
				writeError(w, http.StatusUnauthorized, "invalid admin token")
				return
			}
		}
		next(w, r)
	}
}

// extractPUF извлекает hex-отпечаток из заголовка "Authorization: PUF <hex>".
func extractPUF(r *http.Request) string {
	auth := r.Header.Get("Authorization")
	if after, ok := strings.CutPrefix(auth, "PUF "); ok {
		return strings.TrimSpace(after)
	}
	return ""
}

func writeJSON(w http.ResponseWriter, v any) {
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(v)
}

func writeError(w http.ResponseWriter, code int, msg string) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(code)
	json.NewEncoder(w).Encode(map[string]string{"error": msg})
}
