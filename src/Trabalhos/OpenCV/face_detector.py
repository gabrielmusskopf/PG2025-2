import cv2


class FaceDetector:
    def __init__(self, cascade_path):
        self.face_cascade = cv2.CascadeClassifier(cascade_path)
        if self.face_cascade.empty():
            print(f"Erro: Classificador de face não encontrado em {cascade_path}")
        else:
            print("Detector de face inicializado com sucesso.")

    def detect_faces(self, image):
        """
        Detecta faces na imagem e retorna uma lista de retângulos (x, y, w, h).
        A detecção é realizada em escala de cinza para otimizar a velocidade (crucial no Modo Vídeo).
        """
        if image is None or self.face_cascade.empty():
            return []

        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

        faces = self.face_cascade.detectMultiScale(
            gray,
            scaleFactor=1.1,
            minNeighbors=18,  # Menos do que isso fica muito sensível e dectecta rostos onde não existem
            minSize=(30, 30)
        )

        return faces  # (x, y, w, h)
