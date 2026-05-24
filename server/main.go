// Package main реализует HTTP-сервер puf-server для регистрации
// и аутентификации устройств по их PUF-отпечаткам.
package main

import (
	"context"
	"errors"
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
	// Базовый stderr-логгер для ошибок до того, как уровень станет известен.
	slog.SetDefault(slog.New(slog.NewJSONHandler(os.Stdout, &slog.HandlerOptions{Level: slog.LevelInfo})))

	cfg, err := LoadConfig()
	if err != nil {
		slog.Error("config load", "err", err)
		os.Exit(1)
	}

	if cfg.DatabaseURL == "" {
		slog.Error("config invalid", "err", "DATABASE_URL is required")
		os.Exit(1)
	}
	if cfg.AdminToken == "" {
		slog.Error("config invalid", "err", "ADMIN_TOKEN is required")
		os.Exit(1)
	}

	slog.SetDefault(slog.New(slog.NewJSONHandler(os.Stdout, &slog.HandlerOptions{Level: cfg.LogLevel})))

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	store, err := NewStore(ctx, cfg.DatabaseURL)
	if err != nil {
		slog.Error("store init", "err", err)
		os.Exit(1)
	}
	defer store.Close()

	srv := &Server{store: store, cfg: cfg, metrics: &Metrics{}}
	go srv.gcLimiters(ctx)

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
		slog.Info("shutdown signal received")
		shutCtx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()
		if err := httpSrv.Shutdown(shutCtx); err != nil {
			slog.Error("http shutdown", "err", err)
		}
	}()

	slog.Info("puf-server starting",
		"addr", cfg.ListenAddr,
		"threshold_pct", cfg.ThresholdPct,
		"log_level", cfg.LogLevel.String(),
	)
	if err := httpSrv.ListenAndServe(); err != nil && !errors.Is(err, http.ErrServerClosed) {
		slog.Error("serve", "err", err)
		os.Exit(1)
	}
	slog.Info("puf-server stopped")
}
