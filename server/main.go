// Package main реализует HTTP-сервер puf-server для регистрации
// и аутентификации устройств по их PUF-отпечаткам.
package main

import (
	"context"
	"log"
	"log/slog"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"
)

// main — точка входа сервера: загружает конфигурацию, инициализирует
// хранилище, запускает HTTP-сервер и обрабатывает корректное завершение работы.
func main() {
	cfg, err := LoadConfig()
	if err != nil {
		log.Fatal(err)
	}

	if cfg.DatabaseURL == "" {
		log.Fatal("DATABASE_URL is required")
	}
	if cfg.AdminToken == "" {
		log.Fatal("ADMIN_TOKEN is required")
	}

	slog.SetDefault(slog.New(slog.NewJSONHandler(os.Stdout, nil)))

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	store, err := NewStore(ctx, cfg.DatabaseURL)
	if err != nil {
		log.Fatalf("store init: %v", err)
	}

	srv := &Server{store: store, cfg: cfg}

	httpSrv := &http.Server{
		Addr:              cfg.ListenAddr,
		Handler:           srv.routes(),
		ReadTimeout:       10 * time.Second,
		ReadHeaderTimeout: 5 * time.Second,
		WriteTimeout:      10 * time.Second,
		IdleTimeout:       60 * time.Second,
	}

	go func() {
		<-ctx.Done()
		shutCtx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()
		httpSrv.Shutdown(shutCtx)
	}()

	slog.Info("puf-server starting", "addr", cfg.ListenAddr, "threshold_pct", cfg.ThresholdPct)
	if err := httpSrv.ListenAndServe(); err != nil && err != http.ErrServerClosed {
		log.Fatalf("serve: %v", err)
	}
}
