package main

import (
	"context"
	"net/http"
	"sync"
	"sync/atomic"
	"time"
)

// Metrics хранит счётчики и агрегаты в памяти.
// Все поля обновляются атомарно или под мьютексом.
type Metrics struct {
	EnrollTotal    atomic.Int64
	VerifyOK       atomic.Int64
	VerifyFail     atomic.Int64
	VerifyNotFound atomic.Int64

	mu         sync.Mutex
	hammingSum float64 // сумма hamming_pct всех верификаций для расчёта среднего
	hammingN   int64
}

// RecordVerify фиксирует результат одной верификации.
func (m *Metrics) RecordVerify(ok bool, hammingPct float64) {
	if ok {
		m.VerifyOK.Add(1)
	} else {
		m.VerifyFail.Add(1)
	}
	m.mu.Lock()
	m.hammingSum += hammingPct
	m.hammingN++
	m.mu.Unlock()
}

// AvgHamming возвращает среднее hamming_pct по всем верификациям.
func (m *Metrics) AvgHamming() float64 {
	m.mu.Lock()
	defer m.mu.Unlock()
	if m.hammingN == 0 {
		return 0
	}
	return m.hammingSum / float64(m.hammingN)
}

// handleMetrics отдаёт JSON-снимок всех счётчиков.
func (s *Server) handleMetrics(w http.ResponseWriter, r *http.Request) {
	writeJSON(r.Context(), w, map[string]any{
		"enroll_total":           s.metrics.EnrollTotal.Load(),
		"verify_ok":              s.metrics.VerifyOK.Load(),
		"verify_fail":            s.metrics.VerifyFail.Load(),
		"verify_not_found":       s.metrics.VerifyNotFound.Load(),
		"verify_avg_hamming_pct": s.metrics.AvgHamming(),
	})
}

// handleHealth отвечает 200 OK если сервер жив и БД отвечает на ping.
// Используется Docker healthcheck-ом.
func (s *Server) handleHealth(w http.ResponseWriter, r *http.Request) {
	ctx, cancel := context.WithTimeout(r.Context(), 2*time.Second)
	defer cancel()
	if err := s.store.Ping(ctx); err != nil {
		logger(r.Context()).Warn("health db ping failed", "err", err)
		w.WriteHeader(http.StatusServiceUnavailable)
		writeJSON(r.Context(), w, map[string]string{"status": "db_unavailable"})
		return
	}
	writeJSON(r.Context(), w, map[string]string{"status": "ok"})
}
