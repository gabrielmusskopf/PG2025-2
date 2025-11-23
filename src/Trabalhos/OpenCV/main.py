import cv2
import sys
from data_handler import DataHandler
import image_processor as ip
from filter_config import FILTER_CONFIG
from sticker_manager import StickerManager
from face_detector import FaceDetector

handler = DataHandler()
sticker_manager = StickerManager()
CASCADE_PATH = 'assets/haarcascade_frontalface_default.xml'
face_detector = FaceDetector(CASCADE_PATH)

WINDOW_NAME = 'Aplicativo de Processamento'
image_to_display_global = None
active_video_op_key = None
active_filter_key = None
face_detection_mode_active = False


def display_full_menu():
    """Exibe todos os comandos, incluindo o catálogo de filtros e canais."""
    print("\n--- Comandos do Aplicativo ---")
    print(" L: Carregar Segunda Imagem")
    print(" M: Menu de Operações Matemáticas (Requer Imagem B)")
    print(" T: Seleção de Sticker (Abre Menu de Stickers)")
    print(" R: Resetar operações")
    print(" S: Salvar imagem")
    print(" Q: Sair do aplicativo")
    print("\n--- Catálogo de Filtros ---")

    # Exibe Filtros e Canais
    for key, config in FILTER_CONFIG.items():
        if config['type'] in ['Filter', 'Channel']:
            print(f" {key}: {config['name']}")
    print("------------------------------------------\n")


def load_stickers():
    sticker_manager.load_sticker('Barba', 'assets/sticker_beard.png', offset_y=0.45)
    sticker_manager.load_sticker('Bigode', 'assets/sticker_mustache.png', offset_y=0.2)
    sticker_manager.load_sticker('Óculos', 'assets/sticker_glasses.png', offset_y=-0.1)
    sticker_manager.load_sticker('Óculos de sol', 'assets/sticker_sunglasses.png', offset_y=-0.1, scale=1.1)
    sticker_manager.load_sticker('Nariz de palhaço', 'assets/sticker_clown_nose.png', offset_y=0.1, scale=0.8)
    sticker_manager.load_sticker('Chapéu', 'assets/sticker_hat.png', offset_y=-0.6, scale=1.5)
    sticker_manager.load_sticker('Chapéu de pirata', 'assets/sticker_pirate_hat.png', offset_y=-0.5, scale=1.7)
    sticker_manager.load_sticker('Coroa', 'assets/sticker_crown.png', offset_y=-0.6)


def apply_face_sticker_auto(target_image):
    """
    Tenta aplicar o sticker ativo na primeira face detectada na imagem alvo.
    Retorna (True, imagem_processada) se houver sucesso, ou (False, imagem_original) se falhar.
    """
    if sticker_manager.active_sticker_id is None:
        return False, target_image

    if face_detector.face_cascade.empty():
        print("Erro: Classificador de face não carregado. Não é possível usar o modo automático.")
        return False, target_image

    faces = face_detector.detect_faces(target_image)

    sticker_metadata = sticker_manager.get_sticker_metadata(sticker_manager.active_sticker_id)
    if sticker_metadata is None:
        return False, target_image

    offset_x_meta = sticker_metadata['offset_x']
    offset_y_meta = sticker_metadata['offset_y']
    scale_meta = sticker_metadata['scale']

    processed_image = target_image.copy()

    for (x, y, w, h) in faces:
        # Centro da Face
        center_x = x + (w // 2)
        center_y = y + (h // 2)

        offset_x = int(offset_x_meta * w)
        offset_y = int(offset_y_meta * h)
        target_width = int(w * scale_meta)

        # Redimensionamento do Sticker baseado na largura da face e escala do widget
        sticker_resized_for_face = sticker_manager.get_resized_sticker_for_width(
            sticker_manager.active_sticker_id,
            target_width)

        if sticker_resized_for_face is not None:
            sh, sw, sc = sticker_resized_for_face.shape

            # Ajustar posição com offsets do widget
            adjusted_center_x = center_x + offset_x
            adjusted_center_y = center_y + offset_y

            top_left_x = int(adjusted_center_x - (sw / 2))
            top_left_y = int(adjusted_center_y - (sh / 2))

            # Clampear top_left_x e top_left_y
            # Importante para garantir que o sticker está dento da área visível
            ih, iw, ic = target_image.shape

            top_left_x = max(0, min(top_left_x, iw - 1))
            top_left_y = max(0, min(top_left_y, ih - 1))

            processed_image = sticker_manager.apply_sticker_raw(processed_image, sticker_resized_for_face, top_left_x, top_left_y)

    return True, processed_image


def handle_sticker_selection():
    """Permite ao usuário selecionar um sticker disponível."""
    available_stickers = sticker_manager.get_available_sticker_ids()

    if not available_stickers:
        print("Nenhum sticker carregado no sistema.")
        return

    print("\n--- Seleção de Stickers ---")

    mapping = {}
    for i, sticker_id in enumerate(available_stickers, 1):
        key = str(i)
        mapping[key] = sticker_id
        print(f" {key}: {sticker_id}")

    print(" C: Cancelar / Remover Sticker Ativo")

    choice = input("Digite o número do sticker desejado ou 'C' para remover: ").upper()

    if choice == 'C':
        sticker_manager.clear_active_sticker()
    elif choice in mapping:
        selected_id = mapping[choice]
        sticker_manager.select_sticker(selected_id)
    else:
        print("Opção de sticker inválida.")


def mouse_event(event, x, y, flags, param):
    """Lida com eventos de mouse para posicionamento de stickers (centralizado)."""
    global image_to_display_global

    if sticker_manager.active_sticker is not None:

        if event == cv2.EVENT_LBUTTONDOWN:
            print(f"Clique detectado em ({x}, {y}). Posicionando sticker...")

            if handler.current_image is not None and image_to_display_global is not None:
                # --- A. CÁLCULO DE ESCALA INVERSA ---
                h_disp, w_disp = image_to_display_global.shape[:2]
                h_orig, w_orig = handler.current_image.shape[:2]

                scale_factor_w = w_orig / w_disp
                scale_factor_h = h_orig / h_disp

                # Coordenadas reais do CENTRO do sticker na imagem original
                center_x = int(x * scale_factor_w)
                center_y = int(y * scale_factor_h)

                sh, sw, sc = sticker_manager.active_sticker.shape

                # A posição de início deve ser subtraída pela metade da largura e altura do sticker
                real_x = int(center_x - (sw / 2))
                real_y = int(center_y - (sh / 2))

                # Garantir que as coordenadas não sejam negativas (cortar o sticker na borda)
                real_x = max(0, real_x)
                real_y = max(0, real_y)

                if handler.mode == "Foto":
                    handler.current_image = sticker_manager.apply_sticker(handler.current_image, real_x, real_y)
                    print(f"Sticker aplicado na foto em coordenadas reais ({real_x}, {real_y}).")
                elif handler.mode == "Video":
                    sticker_manager.set_position(real_x, real_y)
                    print(f"Sticker posicionado para persistir no vídeo em coordenadas reais ({real_x}, {real_y}).")


def apply_selected_filter(frame):
    """Busca a função e aplica o filtro ou seleção de canal."""
    if not active_filter_key:
        return frame

    config = FILTER_CONFIG[active_filter_key]

    if config['type'] in ['Filter', 'Channel']:
        func = getattr(ip, config['function'])
        params = config.get('params', {})
        return func(frame, **params)


def apply_selected_math(image_a, image_b):
    """Busca e aplica a função matemática"""
    if not active_video_op_key:
        return image_a

    config = FILTER_CONFIG[active_video_op_key]
    func = getattr(ip, config['function'])
    params = config.get('params', {})
    return func(image_a, image_b, **params)


def scale_image_to_fit(image, max_dim=1000):
    """Redimensiona a imagem se ela for maior que max_dim, mantendo a proporção."""
    if image is None:
        return None

    h, w = image.shape[:2]

    # Se a imagem já for pequena o suficiente, retorna a original
    if w <= max_dim and h <= max_dim:
        return image

    if w > h:
        # Escala baseada na largura
        scale_factor = max_dim / w
    else:
        # Escala baseada na altura
        scale_factor = max_dim / h

    new_w = int(w * scale_factor)
    new_h = int(h * scale_factor)

    return cv2.resize(image, (new_w, new_h), interpolation=cv2.INTER_AREA)


def handle_math_operations():
    if handler.second_image is None:
        print("Erro: Nenhum Frame Congelado (Operando B) carregado. Pressione 'L' para congelar o frame atual.")
        return

    print("\n--- Menu de Operações Matemáticas ---")
    print(" A: Adição Simples (A + B)")
    print(" S: Subtração (A - B) - Útil para detecção de mudança!")
    print(" B: Blending/Mistura (Adição Ponderada, Alpha fixo em 0.5)")
    print(" C: Cancelar e Voltar")

    op_key = input("Selecione a operação: ").upper()

    if op_key == 'A':
        active_math_op = 'M_ADD'
    elif op_key == 'S':
        active_math_op = 'M_SUB'
    elif op_key == 'B':
        active_math_op = 'M_BLD'
    elif op_key == 'C':
        print("Operação cancelada.")
        return
    else:
        print("Opção inválida.")
        return

    global active_video_op_key
    active_video_op_key = active_math_op
    print(f"Operação matemática '{FILTER_CONFIG[active_math_op]['name']}' ativada.")


def process_photo_frame():
    global active_filter_key, active_video_op_key

    if active_filter_key:
        handler.current_image = apply_selected_filter(handler.original_image)
        active_filter_key = None

    if active_video_op_key:
        handler.current_image = apply_selected_math(handler.current_image, handler.second_image)
        active_video_op_key = None


def run_photo_mode(image_path):
    if not handler.load_image(image_path):
        return

    global image_to_display_global, active_filter_key, active_video_op_key
    active_video_op_key = None

    display_full_menu()

    while True:
        if handler.current_image is not None:
            process_photo_frame()
            image_to_display = scale_image_to_fit(handler.current_image, max_dim=900)
            image_to_display_global = image_to_display
            cv2.imshow(WINDOW_NAME, image_to_display)

        key_code = cv2.waitKey(10) & 0xFF
        key_char = chr(key_code).upper()

        if key_char == 'Q':
            break

        # Lógica para Filtros 1-8 e Canais C_R, C_G, C_B (se forem mapeados)
        if key_char in FILTER_CONFIG and FILTER_CONFIG[key_char]['type'] in ['Filter', 'Channel']:
            active_filter_key = key_char
            config = FILTER_CONFIG[active_filter_key]
            print(f"Selecionado: {config['name']}")
            print(f"Descrição: {config['description']}")

        elif key_char == 'L':
            path_b = input("Digite o caminho para a segunda imagem (B): ")
            handler.load_second_image(path_b.strip())

        elif key_char == 'M':
            handle_math_operations()

        elif key_char == 'R':
            active_filter_key = None
            active_video_op_key = None
            handler.reset_operations()
            sticker_manager.clear_active_sticker()

        elif key_char == 'S':
            handler.save_image()

        elif key_char == 'T':
            handle_sticker_selection()
            if sticker_manager.active_sticker_id is not None:
                found, result_image = apply_face_sticker_auto(handler.current_image.copy())
                if found:
                    handler.current_image = result_image
                    sticker_manager.reset_sticker_position()
                    print("Sticker aplicado automaticamente na face detectada.")

    cv2.destroyAllWindows()


def process_video_frame(frame):
    processed_frame = frame.copy()

    found, result_image = apply_face_sticker_auto(processed_frame)
    if found:
        processed_frame = result_image
    elif sticker_manager.sticker_position is not None:
        processed_frame = sticker_manager.apply_sticker(processed_frame)

    if active_filter_key and active_filter_key in FILTER_CONFIG:
        processed_frame = apply_selected_filter(processed_frame)

    if active_video_op_key and handler.second_image is not None:
        processed_frame = apply_selected_math(handler.current_image, handler.second_image)

    return processed_frame


def run_video_mode():
    """Executa o aplicativo no Modo Vídeo (Webcam em tempo real)."""
    if not handler.start_webcam():
        return

    global image_to_display_global, active_video_op_key, active_filter_key
    active_video_op_key = None

    display_full_menu()

    while handler.is_webcam_active:
        frame = handler.read_frame()
        if frame is None:
            break

        processed_frame = process_video_frame(frame)
        image_to_display = scale_image_to_fit(processed_frame, max_dim=900)
        image_to_display_global = image_to_display

        cv2.imshow(WINDOW_NAME, image_to_display)

        key_code = cv2.waitKey(1) & 0xFF
        key_char = chr(key_code).upper()

        if key_char == 'Q':
            break

        if key_char in FILTER_CONFIG and FILTER_CONFIG[key_char]['type'] in ['Filter', 'Channel']:
            active_filter_key = key_char
            config = FILTER_CONFIG[active_filter_key]
            print(f"Selecionado: {config['name']}")
            print(f"Descrição: {config['description']}")

        elif key_char == 'L':
            handler.freeze_current_frame()

        elif key_char == 'M':
            handle_math_operations()

        elif key_char == 'T':
            handle_sticker_selection()

        elif key_char == 'R':
            active_filter_key = None
            active_video_op_key = None
            sticker_manager.clear_active_sticker()
            handler.clear_second_image()
            print("Imagem resetada")

        elif key_char == 'S':
            handler.current_image = processed_frame
            handler.save_image(path=f"frame_capture_{cv2.getTickCount()}.jpg")

    handler.stop_webcam()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    load_stickers()

    cv2.namedWindow(WINDOW_NAME, cv2.WINDOW_AUTOSIZE)
    cv2.setMouseCallback(WINDOW_NAME, mouse_event)

    if len(sys.argv) > 1:
        # Se um caminho de imagem for fornecido, inicia o Modo Foto
        image_file = sys.argv[-1]
        run_photo_mode(image_file)
    else:
        # Se nenhum argumento for fornecido, inicia o Modo Vídeo
        print("Nenhuma imagem fornecida. Iniciando Modo Vídeo (Webcam)...")
        run_video_mode()
