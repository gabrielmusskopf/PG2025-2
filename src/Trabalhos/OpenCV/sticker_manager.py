import cv2
import numpy as np

MAX_STICKER_WIDTH = 200


class StickerManager:
    def __init__(self):
        self.sticker_base = {}
        self.active_sticker = None
        self.sticker_position = None
        self.active_sticker_id = None

    def load_sticker(self, sticker_id, path, offset_x=0.0, offset_y=0.0, scale=1.0, ref_point='center'):
        """
        Carrega um sticker (PNG com canal alfa) com metadados de offset para posicionamento automático.
        Requisito: Mínimo 5 stickers diferentes [3].
        """
        img = cv2.imread(path, cv2.IMREAD_UNCHANGED)

        if img is not None and img.shape[2] == 4:  # Verifica se possui 4 canais (BGRA)
            self.sticker_base[sticker_id] = {
                'image': img,
                'offset_x': offset_x,
                'offset_y': offset_y,
                'scale': scale,
                'ref_point': ref_point
            }
            print(f"Sticker '{sticker_id}' carregado com offset: ({offset_x}, {offset_y}) e escala {scale}")
            return True
        else:
            if img is None:
                print(f"Erro ao carregar sticker em: {path}")
            else:
                print(f"Erro: Sticker '{sticker_id}' não possui canal alfa (transparência).")
            return False

    def _resize_sticker(self, sticker_img, target_width):
        """Redimensiona o sticker mantendo a proporção com base na largura alvo, permitindo scale up ou scale down."""
        h, w = sticker_img.shape[:2]

        if w == target_width:
            return sticker_img

        # Cálculo do fator de escala
        scale_factor = target_width / w
        new_w = target_width
        new_h = int(h * scale_factor)

        # Seleção da Interpolação para Qualidade
        # INTER_AREA é geralmente o melhor para REDUÇÃO de tamanho (diminuir artefatos).
        # INTER_LINEAR ou INTER_CUBIC são geralmente melhores para AUMENTO (scale up).
        if scale_factor < 1.0:
            # Scale Down (redução)
            interpolation = cv2.INTER_AREA
        else:
            # Scale Up (ampliação)
            interpolation = cv2.INTER_LINEAR  # Bom equilíbrio entre qualidade e velocidade

        return cv2.resize(sticker_img, (new_w, new_h), interpolation=interpolation)

    def get_available_sticker_ids(self):
        """Retorna a lista de IDs de stickers carregados."""
        return list(self.sticker_base.keys())

    def get_sticker_metadata(self, sticker_id):
        """Retorna os metadados completos do sticker."""
        return self.sticker_base.get(sticker_id)

    def get_resized_sticker_for_width(self, sticker_id, target_width):
        """Retorna a imagem do sticker redimensionada para uma largura alvo (usado na detecção de face)."""
        metadata = self.get_sticker_metadata(sticker_id)
        if metadata:
            return self._resize_sticker(metadata['image'], target_width)
        return None

    def select_sticker(self, sticker_id):
        """Seleciona o sticker ativo e define a versão redimensionada para uso manual."""
        metadata = self.get_sticker_metadata(sticker_id)
        if metadata:
            # Versão redimensionada para uso manual (MAX_STICKER_WIDTH)
            self.active_sticker = self._resize_sticker(metadata['image'], MAX_STICKER_WIDTH)

            self.active_sticker_id = sticker_id
            self.sticker_position = None
            print(f"Sticker '{sticker_id}' selecionado (Largura máx. {MAX_STICKER_WIDTH}px).")
        else:
            self.active_sticker = None
            self.active_sticker_id = None
            print(f"Sticker '{sticker_id}' não encontrado na base.")

    def clear_active_sticker(self):
        """Limpa o sticker ativo e a posição."""
        self.active_sticker = None
        self.active_sticker_id = None
        self.sticker_position = None
        print("Sticker ativo removido.")

    def reset_sticker_position(self):
        """Remove a posição persistente do sticker (usado no Resetar e Detecção de Face)."""
        self.sticker_position = None
        print("Posição de sticker resetada.")

    def set_position(self, x, y):
        """Define a posição de aplicação do sticker (coordenadas Top-Left)."""
        self.sticker_position = (x, y)

    def apply_sticker(self, target_image, top_left_x=None, top_left_y=None):
        """
        Aplica o sticker ativo (pré-redimensionado para uso manual) na imagem alvo.
        Usado para posicionamento manual via clique.
        """
        if self.active_sticker is None:
            return target_image

        # Prioriza coordenadas passadas (se houver), senão usa a posição persistente
        x_start, y_start = self.sticker_position if self.sticker_position else (top_left_x, top_left_y)

        if x_start is None or y_start is None:
            return target_image

        # Reutiliza a lógica central de aplicação
        return self.apply_sticker_raw(target_image, self.active_sticker, x_start, y_start)

    def apply_sticker_raw(self, target_image, sticker_img, top_left_x, top_left_y):
        """
        Aplica um sticker (que já deve estar redimensionado) na imagem alvo.
        Usado internamente pela função apply_sticker e pelo modo Detecção de Face.
        """
        if target_image is None or sticker_img is None:
            return target_image

        sh, sw, sc = sticker_img.shape
        ih, iw, ic = target_image.shape

        x_start = top_left_x
        y_start = top_left_y

        x_end = min(x_start + sw, iw)
        y_end = min(y_start + sh, ih)

        sticker_w = x_end - x_start
        sticker_h = y_end - y_start

        if sticker_w <= 0 or sticker_h <= 0:
            return target_image

        # Ajustar Sticker e Alfa
        sticker_roi = sticker_img[0:sticker_h, 0:sticker_w]
        sticker_bgr = sticker_roi[:, :, 0:3]
        alpha = sticker_roi[:, :, 3] / 255.0
        alpha_inv = 1.0 - alpha

        # Obter ROI e Aplicar Blending
        roi = target_image[y_start:y_end, x_start:x_end]

        img_background = (roi * alpha_inv[:, :, np.newaxis]).astype(target_image.dtype)
        img_foreground = (sticker_bgr * alpha[:, :, np.newaxis]).astype(target_image.dtype)

        final_roi = cv2.add(img_background, img_foreground)

        # Inserir a ROI final
        target_image[y_start:y_end, x_start:x_end] = final_roi

        return target_image
