package main

import (
	"fmt"
	"log/slog"
	"os"
	"strconv"
)

// Config хранит все параметры запуска, считанные из переменных окружения.
type Config struct {
	ListenAddr   string     // LISTEN_ADDR, по умолчанию ":8080"
	DatabaseURL  string     // DATABASE_URL (обязательный параметр)
	AdminToken   string     // ADMIN_TOKEN (required)
	ThresholdPct float64    // PUF_THRESHOLD_PCT, по умолчанию 10.0 — максимальный допустимый intra-HD в процентах
	LogLevel     slog.Level // LOG_LEVEL: debug|info|warn|error, по умолчанию info
}

// LoadConfig читает конфигурацию из переменных окружения.
func LoadConfig() (Config, error) {
	threshold := 10.0
	if s := os.Getenv("PUF_THRESHOLD_PCT"); s != "" {
		v, err := strconv.ParseFloat(s, 64)
		if err != nil {
			return Config{}, fmt.Errorf("invalid PUF_THRESHOLD_PCT: %w", err)
		}
		threshold = v
	}
	if threshold < 0.0 || threshold > 50.0 {
		return Config{}, fmt.Errorf("PUF_THRESHOLD_PCT must be in [0, 50]")
	}

	level := slog.LevelInfo
	if s := os.Getenv("LOG_LEVEL"); s != "" {
		if err := level.UnmarshalText([]byte(s)); err != nil {
			return Config{}, fmt.Errorf("invalid LOG_LEVEL %q (expected debug|info|warn|error)", s)
		}
	}

	return Config{
		ListenAddr:   getenv("LISTEN_ADDR", ":8080"),
		DatabaseURL:  os.Getenv("DATABASE_URL"),
		AdminToken:   os.Getenv("ADMIN_TOKEN"),
		ThresholdPct: threshold,
		LogLevel:     level,
	}, nil
}

func getenv(key, def string) string {
	if v := os.Getenv(key); v != "" {
		return v
	}
	return def
}
