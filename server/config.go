package main

import (
	"os"
	"strconv"
)

// Config хранит все параметры запуска, считанные из переменных окружения.
type Config struct {
	ListenAddr   string  // LISTEN_ADDR, по умолчанию ":8080"
	DatabaseURL  string  // DATABASE_URL (обязательный параметр)
	AdminToken   string  // ADMIN_TOKEN; если не задан — админ-эндпоинты открыты
	ThresholdPct float64 // PUF_THRESHOLD_PCT, по умолчанию 10.0 — максимальный допустимый intra-HD в процентах
}

// LoadConfig читает конфигурацию из переменных окружения.
func LoadConfig() Config {
	threshold := 10.0
	if s := os.Getenv("PUF_THRESHOLD_PCT"); s != "" {
		if v, err := strconv.ParseFloat(s, 64); err == nil {
			threshold = v
		}
	}
	return Config{
		ListenAddr:   getenv("LISTEN_ADDR", ":8080"),
		DatabaseURL:  os.Getenv("DATABASE_URL"),
		AdminToken:   os.Getenv("ADMIN_TOKEN"),
		ThresholdPct: threshold,
	}
}

func getenv(key, def string) string {
	if v := os.Getenv(key); v != "" {
		return v
	}
	return def
}
