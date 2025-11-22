import numpy as np
import cv2 as cv

# Carregar imagens
img1 = cv.imread('./baboon.png') # Fundo
img2 = cv.imread('./eyeglasses.png', cv.IMREAD_UNCHANGED) # Overlay (com Alpha)
img3 = cv.imread('./eyeglasses.png') # Overlay (RGB puro)

assert img1 is not None and img2 is not None
b, g, r, a = cv.split(img2)

# Dimensões
rows_bg, cols_bg, _ = img1.shape
rows_fg, cols_fg, _ = img3.shape

# --- 1. DEFINIR COORDENADAS PELO CENTRO ---
# Vamos simular um clique do mouse em (x, y)
center_x, center_y = 256, 70

# Calculamos o topo-esquerdo (Top-Left) subtraindo metade da largura/altura
tl_x = center_x - int(cols_fg / 2)
tl_y = center_y - int(rows_fg / 2)

# Calculamos o fundo-direito (Bottom-Right)
br_x = tl_x + cols_fg
br_y = tl_y + rows_fg

# --- 2. CLIPPING (CORTE DE SEGURANÇA) ---
# Precisamos de índices para a imagem de FUNDO (bg) e para a imagem de OVERLAY (fg)

# Inicializamos os índices do Overlay como a imagem inteira
fg_x1, fg_y1 = 0, 0
fg_x2, fg_y2 = cols_fg, rows_fg

# Inicializamos os índices do Fundo com as posições calculadas
bg_x1, bg_y1 = tl_x, tl_y
bg_x2, bg_y2 = br_x, br_y

# [Eixo X] Verifica se saiu pela ESQUERDA
if bg_x1 < 0:
    diff = abs(bg_x1) # Quantos pixels saíram para fora?
    bg_x1 = 0         # Fundo começa no 0
    fg_x1 = diff      # Overlay começa 'diff' pixels para dentro

# [Eixo X] Verifica se saiu pela DIREITA
if bg_x2 > cols_bg:
    bg_x2 = cols_bg
    # O novo fim do overlay é o inicio + a nova largura válida
    fg_x2 = fg_x1 + (bg_x2 - bg_x1) 

# [Eixo Y] Verifica se saiu por CIMA
if bg_y1 < 0:
    diff = abs(bg_y1)
    bg_y1 = 0
    fg_y1 = diff

# [Eixo Y] Verifica se saiu por BAIXO
if bg_y2 > rows_bg:
    bg_y2 = rows_bg
    fg_y2 = fg_y1 + (bg_y2 - bg_y1)

# --- 3. VERIFICAÇÃO FINAL E APLICAÇÃO ---
# Calculamos a largura e altura finais da região válida
valid_w = bg_x2 - bg_x1
valid_h = bg_y2 - bg_y1

# Só processa se sobrar alguma imagem visível (largura e altura > 0)
if valid_w > 0 and valid_h > 0:
    # Recortamos as fatias EXATAS de cada imagem
    roi_bg = img1[bg_y1:bg_y2, bg_x1:bg_x2]
    roi_fg = img3[fg_y1:fg_y2, fg_x1:fg_x2]
    
    # Recortamos também a máscara (Alpha) para bater com o tamanho
    mask_valid = a[fg_y1:fg_y2, fg_x1:fg_x2]
    mask_inv_valid = cv.bitwise_not(mask_valid)

    # --- OPERAÇÃO BITWISE (A mesma de antes, mas nas fatias recortadas) ---
    img1_bg = cv.bitwise_and(roi_bg, roi_bg, mask=mask_inv_valid)
    img2_fg = cv.bitwise_and(roi_fg, roi_fg, mask=mask_valid)
    
    dst = cv.add(img1_bg, img2_fg)
    
    # Colocamos o resultado de volta na região válida do fundo
    img1[bg_y1:bg_y2, bg_x1:bg_x2] = dst

cv.imshow('Resultado Centralizado com Clipping', img1)
cv.waitKey(0)
cv.destroyAllWindows()