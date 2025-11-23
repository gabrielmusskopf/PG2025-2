"""
Módulo com a lógica de processamento de imagem
"""
import cv2
import numpy as np


def apply_smoothing_filter(image, kernel_size=(5, 5)):
    """Aplica um filtro de Suavização (Gaussian Blur) na imagem."""
    if image is None:
        return None
    return cv2.GaussianBlur(image, kernel_size, 0)


def apply_enhancement_filter(image):
    """Aplica um filtro de Realce (Sharpen)."""
    if image is None:
        return None

    # Kernel básico para Sharpen (para um realce mais perceptível)
    kernel = np.array([[-1, -1, -1],
                       [-1, 9, -1],
                       [-1, -1, -1]])

    return cv2.filter2D(image, -1, kernel)


def apply_canny_filter(image, t_low=100, t_high=200):
    """Detecção de Bordas Canny."""
    if image is None:
        return None

    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    edges = cv2.Canny(gray, t_low, t_high)
    return cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)


def apply_inversion_filter(image):
    """Inversão de Cores."""
    if image is None:
        return None
    return cv2.bitwise_not(image)


def apply_thresholding(image, threshold=127):
    """Binarização Simples."""
    if image is None:
        return None
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    _, binary = cv2.threshold(gray, threshold, 255, cv2.THRESH_BINARY)
    return cv2.cvtColor(binary, cv2.COLOR_GRAY2BGR)


def apply_erosion(image, kernel_size=(5, 5)):
    """Erosão Morfológica."""
    if image is None:
        return None
    kernel = np.ones(kernel_size, np.uint8)
    return cv2.erode(image, kernel, iterations=1)


def apply_dilation(image, kernel_size=(5, 5)):
    """Dilatação Morfológica."""
    if image is None:
        return None
    kernel = np.ones(kernel_size, np.uint8)
    return cv2.dilate(image, kernel, iterations=1)


def adjust_brightness(image, gamma=50):
    """Ajuste de Brilho (Adiciona/Subtrai um valor constante)."""
    if image is None:
        return None
    # Adição ponderada onde o segundo termo é uma matriz constante de brilho/gamma
    brightness_matrix = np.ones(image.shape, dtype="uint8") * abs(gamma)

    if gamma > 0:
        # Aumentar brilho
        return cv2.add(image, brightness_matrix)
    elif gamma < 0:
        # Diminuir brilho
        return cv2.subtract(image, brightness_matrix)
    else:
        return image


def select_channel(image, channel='R'):
    """Exibe um único canal de cor (R, G ou B)."""
    if image is None or image.ndim < 3:
        return image

    # Canais no OpenCV são geralmente BGR
    b, g, r = cv2.split(image)

    if channel.upper() == 'R':
        target_channel = r
    elif channel.upper() == 'G':
        target_channel = g
    elif channel.upper() == 'B':
        target_channel = b
    else:
        return image

    zero_channel = np.zeros_like(target_channel)

    if channel.upper() == 'R':
        # Cria uma imagem onde apenas o canal R tem intensidade
        result_image = cv2.merge([zero_channel, zero_channel, target_channel])
    elif channel.upper() == 'G':
        result_image = cv2.merge([zero_channel, target_channel, zero_channel])
    else:  # B
        result_image = cv2.merge([target_channel, zero_channel, zero_channel])

    return result_image


def ensure_same_size(image1, image2):
    """Garante que a segunda imagem tenha o mesmo tamanho que a primeira."""
    if image1 is None or image2 is None:
        return None, None

    h1, w1 = image1.shape[:2]
    image2_resized = cv2.resize(image2, (w1, h1), interpolation=cv2.INTER_AREA)
    return image1, image2_resized


def apply_add(image_a, image_b):
    if image_a is None or image_b is None:
        print("Erro: Ambas as imagens devem ser carregadas para operações aritméticas.")
        return image_a

    img_a, img_b = ensure_same_size(image_a, image_b)

    result = cv2.add(img_a, img_b)
    print("Operação: Adição de Imagens concluída.")
    return result


def apply_sub(image_a, image_b):
    if image_a is None or image_b is None:
        print("Erro: Ambas as imagens devem ser carregadas para operações aritméticas.")
        return image_a

    img_a, img_b = ensure_same_size(image_a, image_b)

    result = cv2.subtract(img_a, img_b)
    print("Operação: Subtração de Imagens concluída.")
    return result


def apply_blend(image_a, image_b, alpha=0.5):
    if image_a is None or image_b is None:
        print("Erro: Ambas as imagens devem ser carregadas para operações aritméticas.")
        return image_a

    img_a, img_b = ensure_same_size(image_a, image_b)

    beta = 1.0 - alpha
    # blend = (img_a * alpha) + (img_b * beta) + gamma (gamma=0, beta=1-alpha)
    result = cv2.addWeighted(img_a, alpha, img_b, beta, 0)
    print(f"Operação: Blending concluída (Peso A={alpha}, Peso B={beta:.1f}).")
    return result
