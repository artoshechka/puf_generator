// Пакет main реализует сервер верификации PUF-отпечатков.
// Устройство аутентифицируется, отправляя свежий PUF-отпечаток (hex-строку)
// в заголовке Authorization; сервер сравнивает его с эталоном по расстоянию Хэмминга
// с настраиваемым порогом допустимого отклонения.
package main

import (
	"encoding/hex"
	"fmt"
	"math/bits"
)

// parseHex декодирует hex-строку в байты и отклоняет пустой ввод.
// Если expectedBytes > 0, длина должна совпадать.
func parseHex(s string, expectedBytes int) ([]byte, error) {
	b, err := hex.DecodeString(s)
	if err != nil {
		return nil, fmt.Errorf("invalid hex fingerprint: %w", err)
	}
	if len(b) == 0 {
		return nil, fmt.Errorf("fingerprint must not be empty")
	}
	if expectedBytes > 0 && len(b) != expectedBytes {
		return nil, fmt.Errorf("fingerprint length mismatch: %d vs %d", len(b), expectedBytes)
	}
	return b, nil
}

// hammingDistance считает количество различающихся бит между a и b.
// Требует совпадения длины.
func hammingDistance(a, b []byte) (int, error) {
	if len(a) != len(b) {
		return 0, fmt.Errorf("fingerprint length mismatch: %d vs %d", len(a), len(b))
	}
	dist := 0
	for i := range a {
		dist += bits.OnesCount8(a[i] ^ b[i])
	}
	return dist, nil
}

// fractionalHD возвращает расстояние Хэмминга на уровне бит, нормированное к [0.0, 1.0].
// Умножьте на 100, чтобы получить процент для сравнения с порогом.
func fractionalHD(a, b []byte) (float64, error) {
	if len(a) == 0 || len(b) == 0 {
		return 0, fmt.Errorf("fingerprint must not be empty")
	}
	dist, err := hammingDistance(a, b)
	if err != nil {
		return 0, err
	}
	totalBits := len(a) * 8
	return float64(dist) / float64(totalBits), nil
}
