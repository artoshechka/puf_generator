package main

import (
	"context"
	"fmt"
	"time"

	"github.com/jackc/pgx/v5/pgxpool"
)

// Device — запись о зарегистрированном устройстве в PostgreSQL.
type Device struct {
	ID             string
	FingerprintHex string
	EnrolledAt     time.Time
}

// Store оборачивает пул соединений pgxpool и обеспечивает хранение отпечатков.
type Store struct {
	pool *pgxpool.Pool
}

// NewStore подключается к PostgreSQL и выполняет миграцию схемы перед возвратом.
func NewStore(ctx context.Context, dsn string) (*Store, error) {
	pool, err := pgxpool.New(ctx, dsn)
	if err != nil {
		return nil, fmt.Errorf("connect: %w", err)
	}
	s := &Store{pool: pool}
	if err := s.migrate(ctx); err != nil {
		pool.Close()
		return nil, fmt.Errorf("migrate: %w", err)
	}
	return s, nil
}

// migrate создаёт таблицу devices, если она ещё не существует.
func (s *Store) migrate(ctx context.Context) error {
	_, err := s.pool.Exec(ctx, `
		CREATE TABLE IF NOT EXISTS devices (
			id             TEXT        PRIMARY KEY,
			fingerprint_hex TEXT       NOT NULL,
			enrolled_at    TIMESTAMPTZ NOT NULL DEFAULT NOW()
		)
	`)
	return err
}

// Enroll сохраняет эталонный отпечаток устройства, заменяя предыдущую регистрацию.
func (s *Store) Enroll(ctx context.Context, id, fpHex string) error {
	_, err := s.pool.Exec(ctx, `
		INSERT INTO devices (id, fingerprint_hex)
		VALUES ($1, $2)
		ON CONFLICT (id) DO UPDATE
			SET fingerprint_hex = EXCLUDED.fingerprint_hex,
			    enrolled_at     = NOW()
	`, id, fpHex)
	return err
}

// Get возвращает запись устройства; при отсутствии возвращает pgx.ErrNoRows.
func (s *Store) Get(ctx context.Context, id string) (*Device, error) {
	var d Device
	err := s.pool.QueryRow(ctx,
		`SELECT id, fingerprint_hex, enrolled_at FROM devices WHERE id = $1`, id,
	).Scan(&d.ID, &d.FingerprintHex, &d.EnrolledAt)
	if err != nil {
		return nil, err
	}
	return &d, nil
}

// List возвращает все зарегистрированные устройства, отсортированные по времени регистрации.
func (s *Store) List(ctx context.Context) ([]Device, error) {
	rows, err := s.pool.Query(ctx,
		`SELECT id, fingerprint_hex, enrolled_at FROM devices ORDER BY enrolled_at`,
	)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var out []Device
	for rows.Next() {
		var d Device
		if err := rows.Scan(&d.ID, &d.FingerprintHex, &d.EnrolledAt); err != nil {
			return nil, err
		}
		out = append(out, d)
	}
	return out, rows.Err()
}

// Delete удаляет устройство; возвращает false, если запись не найдена.
func (s *Store) Delete(ctx context.Context, id string) (bool, error) {
	tag, err := s.pool.Exec(ctx, `DELETE FROM devices WHERE id = $1`, id)
	if err != nil {
		return false, err
	}
	return tag.RowsAffected() > 0, nil
}

// Ping проверяет, что соединение с БД живо.
func (s *Store) Ping(ctx context.Context) error {
	return s.pool.Ping(ctx)
}

// Close освобождает пул соединений. Вызывается при остановке сервера.
func (s *Store) Close() {
	s.pool.Close()
}
