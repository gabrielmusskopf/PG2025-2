"""
Módulo que lida com o carregamento e o salvamento.
"""
import cv2
import os


class DataHandler:
    def __init__(self):
        self.current_image = None
        self.original_image = None
        self.second_image = None
        self.mode = "Foto"
        self.cap = None
        self.is_webcam_active = False

    def load_image(self, path):
        """Carrega a imagem principal (A) do disco."""
        img = cv2.imread(path)
        if img is not None:
            self.original_image = img.copy()
            self.current_image = img.copy()
            self.mode = "Foto"
            print(f"Imagem principal '{path}' carregada com sucesso.")
            return True
        else:
            print(f"Erro ao carregar imagem principal em: {path}")
            return False

    def load_second_image(self, path):
        """Carrega a segunda imagem (B) do disco para operações matemáticas."""
        img = cv2.imread(path)
        if img is not None:
            self.second_image = img.copy()
            print(f"Segunda imagem '{path}' carregada com sucesso.")
            return True
        else:
            print(f"Erro ao carregar segunda imagem em: {path}")
            self.second_image = None
            return False

    def save_image(self, path="output_image.jpg"):
        """Salva a imagem atualmente processada."""
        output_folder = "captures"
        if not os.path.exists(output_folder):
            os.makedirs(output_folder)

        path = os.path.join(output_folder, path)
        if self.current_image is not None:
            cv2.imwrite(path, self.current_image)
            print(f"Imagem salva em: {path}")
            return True
        return False

    def reset_operations(self):
        """Volta à imagem original [2]."""
        if self.original_image is not None:
            self.current_image = self.original_image.copy()
            print("Operações resetadas.")
            return True
        return False

    def start_webcam(self, camera_index=0):
        """Inicia a captura de vídeo da webcam."""
        self.cap = cv2.VideoCapture(camera_index)

        if not self.cap.isOpened():
            print("Erro: Não foi possível acessar a webcam.")
            self.is_webcam_active = False
            return False

        self.mode = "Video"
        self.is_webcam_active = True
        print("Modo Vídeo iniciado (Webcam). Pressione 'Q' para sair.")
        return True

    def stop_webcam(self):
        """Libera a webcam e atualiza o estado."""
        if self.cap is not None:
            self.cap.release()
            self.is_webcam_active = False
            self.cap = None
            print("Modo Vídeo encerrado.")

    def read_frame(self):
        """Lê um frame da webcam."""
        if self.is_webcam_active and self.cap.isOpened():
            ret, frame = self.cap.read()
            if ret:
                self.current_image = frame
                self.original_image = frame.copy()
                return frame
        return None

    def freeze_current_frame(self):
        """Salva o current_image no slot second_image (Operando B)."""
        if self.current_image is not None:
            self.second_image = self.current_image.copy()
            print("Frame congelado com sucesso para operações matemáticas (operando B).")
            return True
        return False

    def clear_second_image(self):
        """Limpa a segunda imagem (usada em operações matemáticas)"""
        self.second_image = None
