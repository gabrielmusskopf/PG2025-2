FILTER_CONFIG = {
    # --- Filtros ---
    '1': {
        'name': 'Suavização Gaussiana (Blur)',
        'type': 'Filter',
        'description': 'Reduz ruídos na imagem e suaviza transições. Requer a configuração do tamanho do kernel (impar).',
        'function': 'apply_smoothing_filter',
        'params': {'kernel_size': (5, 5)}
    },
    '2': {
        'name': 'Realce (Sharpen)',
        'type': 'Filter',
        'description': 'Aplica um kernel de convolução para aumentar a nitidez das bordas e detalhes da imagem.',
        'function': 'apply_enhancement_filter',
        'params': {}
    },
    '3': {
        'name': 'Detecção de Bordas (Canny)',
        'type': 'Filter',
        'description': 'Aplica o algoritmo Canny para identificar bordas fortes e fracas. Configuração de dois thresholds (limiares) é possível.',
        'function': 'apply_canny_filter',
        'params': {'t_low': 100, 't_high': 200}
    },
    '4': {
        'name': 'Inversão de Cores',
        'type': 'Filter',
        'description': 'Inverte os valores de pixel (255 - valor), resultando em um negativo da imagem.',
        'function': 'apply_inversion_filter',
        'params': {}
    },
    '5': {
        'name': 'Binarização (Threshold Simples)',
        'type': 'Filter',
        'description': 'Converte a imagem para preto e branco, baseando-se em um valor de limiar. A imagem é convertida para Grayscale primeiro.',
        'function': 'apply_thresholding',
        'params': {'threshold': 127}
    },
    '6': {
        'name': 'Erosão Morfológica',
        'type': 'Filter',
        'description': 'Filtro morfológico que "desgasta" as bordas de objetos em primeiro plano, diminuindo-os.',
        'function': 'apply_erosion',
        'params': {'kernel_size': (5, 5)}
    },
    '7': {
        'name': 'Dilatação Morfológica',
        'type': 'Filter',
        'description': 'Filtro morfológico que "engrossa" as bordas de objetos em primeiro plano.',
        'function': 'apply_dilation',
        'params': {'kernel_size': (5, 5)}
    },
    '8': {
        'name': 'Ajuste de Brilho',
        'type': 'Filter',
        'description': 'Aumenta ou diminui o brilho da imagem usando adição (gamma). Valor deve ser entre -100 e 100.',
        'function': 'adjust_brightness',
        'params': {'gamma': 50}
    },

    # --- Operações Matemáticas ---
    'M_ADD': {
        'name': 'Adição de Imagens (A + B)',
        'type': 'Operation',
        'description': 'Soma os valores de pixel das imagens A e B. Imagens precisam ter o mesmo tamanho.',
        'function': 'apply_add',
        'params': {}
    },
    'M_SUB': {
        'name': 'Subtração de Imagens (A - B)',
        'type': 'Operation',
        'description': 'Subtrai os valores de pixel da Imagem B da Imagem A. Útil para detecção de diferenças.',
        'function': 'apply_sub',
        'params': {}
    },
    'M_BLD': {
        'name': 'Blending (Mistura Ponderada)',
        'type': 'Operation',
        'description': 'Mistura Imagem A e Imagem B usando pesos (Alpha e Beta). Permite controlar a transparência da mistura.',
        'function': 'apply_blend',
        'params': {'alpha': 0.5},
    },

    # --- Seleção de Canais ---
    '9': {
        'name': 'Exibir Canal Vermelho (R)',
        'type': 'Channel',
        'description': 'Exibe a intensidade do canal vermelho da imagem. Não é um filtro, mas uma visualização.',
        'function': 'select_channel',
        'params': {'channel': 'R'}
    },
}
