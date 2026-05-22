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
func parseHex(s string) ([]byte, error) {
	b, err := hex.DecodeString(s)
	if err != nil {
		return nil, fmt.Errorf("invalid hex fingerprint: %w", err)
	}
	if len(b) == 0 {
		return nil, fmt.Errorf("fingerprint must not be empty")
	}
	return b, nil
}

// hammingDistance считает количество различающихся бит между a и b.
// Если длины отличаются, сравнивается только перекрывающийся префикс.
func hammingDistance(a, b []byte) int {
	n := min(len(a), len(b))
	dist := 0
	for i := range n {
		dist += bits.OnesCount8(a[i] ^ b[i])
	}
	return dist
}

// fractionalHD возвращает расстояние Хэмминга на уровне бит, нормированное к [0.0, 1.0].
// Умножьте на 100, чтобы получить процент для сравнения с порогом.
func fractionalHD(a, b []byte) float64 {
	if len(a) == 0 || len(b) == 0 {
		return 0
	}
	totalBits := min(len(a), len(b)) * 8
	return float64(hammingDistance(a, b)) / float64(totalBits)
}
