package main

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"log/slog"
	"net/http"
	"time"
)

// ctxKey — приватный тип для ключей в context.Context, чтобы избежать коллизий.
type ctxKey int

const (
	ctxKeyReqID ctxKey = iota
)

// responseWriter оборачивает http.ResponseWriter чтобы перехватить статус-код
// и подсчитать объём отданного тела.
type responseWriter struct {
	http.ResponseWriter
	status    int
	bytesSent int64
}

func (rw *responseWriter) WriteHeader(code int) {
	rw.status = code
	rw.ResponseWriter.WriteHeader(code)
}

func (rw *responseWriter) Write(p []byte) (int, error) {
	if rw.status == 0 {
		rw.status = http.StatusOK
	}
	n, err := rw.ResponseWriter.Write(p)
	rw.bytesSent += int64(n)
	return n, err
}

func reqID() string {
	b := make([]byte, 4)
	if _, err := rand.Read(b); err != nil {
		return "00000000"
	}
	return hex.EncodeToString(b)
}

// reqIDFromCtx возвращает request ID из контекста или пустую строку.
func reqIDFromCtx(ctx context.Context) string {
	if v, ok := ctx.Value(ctxKeyReqID).(string); ok {
		return v
	}
	return ""
}

// logger возвращает slog.Logger с привязанным request ID из контекста.
// Все handler-логи должны идти через него — иначе теряется корреляция с requestLogger.
func logger(ctx context.Context) *slog.Logger {
	if id := reqIDFromCtx(ctx); id != "" {
		return slog.Default().With("req_id", id)
	}
	return slog.Default()
}

// requestLogger — middleware, логирует каждый HTTP-запрос с request ID, методом,
// путём, статусом, временем выполнения и объёмами тела.
func requestLogger(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		id := reqID()

		w.Header().Set("X-Request-ID", id)
		rw := &responseWriter{ResponseWriter: w, status: http.StatusOK}

		ctx := context.WithValue(r.Context(), ctxKeyReqID, id)
		next.ServeHTTP(rw, r.WithContext(ctx))

		slog.Info("request",
			"req_id", id,
			"method", r.Method,
			"path", r.URL.Path,
			"status", rw.status,
			"latency_ms", time.Since(start).Milliseconds(),
			"remote", r.RemoteAddr,
			"user_agent", r.UserAgent(),
			"bytes_in", r.ContentLength,
			"bytes_out", rw.bytesSent,
		)
	})
}
