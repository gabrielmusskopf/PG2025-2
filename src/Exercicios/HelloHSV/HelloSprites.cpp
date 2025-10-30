/*
 * Hello Triangle - Código adaptado de:
 *   - https://learnopengl.com/#!Getting-started/Hello-Triangle
 *   - https://antongerdelan.net/opengl/glcontext2.html
 *
 * Adaptado por: Rossana Baptista Queiroz
 *
 * Disciplinas:
 *   - Processamento Gráfico (Ciência da Computação - Híbrido)
 *   - Processamento Gráfico: Fundamentos (Ciência da Computação - Presencial)
 *   - Fundamentos de Computação Gráfica (Jogos Digitais)
 *
 * Descrição:
 *   Este código é o "Olá Mundo" da Computação Gráfica, utilizando OpenGL Moderna.
 *   No pipeline programável, o desenvolvedor pode implementar as etapas de
 *   Processamento de Geometria e Processamento de Pixel utilizando shaders.
 *   Um programa de shader precisa ter, obrigatoriamente, um Vertex Shader e um Fragment Shader,
 *   enquanto outros shaders, como o de geometria, são opcionais.
 *
 * Histórico:
 *   - Versão inicial: 07/04/2017
 *   - Última atualização: 18/03/2025
 *
 */

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

// STB_IMAGE
// STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Sprite.h"

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
GLuint loadTexture(string filePath);
void processInput(Sprite &spr);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"glsl(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec2 texc;
 out vec2 tex_coord; 
 uniform mat4 projection;
 uniform mat4 model;
 void main()
 {
	 gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);
	 tex_coord = vec2(1-texc.s,texc.t);
 }
 )glsl";

// Código fonte do Fragment Shader (com a lógica HSV pedagógica)
const GLchar *fragmentShaderSource = R"glsl(
    #version 400
    
    in vec2 tex_coord; // Recebido do Vertex Shader
    out vec4 FragColor;

    // Uniforms para Saturação e Valor
    uniform float u_saturation;
    uniform float u_value;

    /*
     * Converte HSV para RGB usando a lógica matemática explícita,
     * baseada em setores (if/else), ideal para ensino.
     */
    vec3 hsv2rgb_pedagogical(vec3 c) {
        float H = c.x;
        float S = c.y;
        float V = c.z;
        float C = V * S;
        float H_seis = H * 6.0;
        float X = C * (1.0 - abs(mod(H_seis, 2.0) - 1.0));
        vec3 rgb_prime;

        if (H_seis >= 0.0 && H_seis < 1.0) {
            rgb_prime = vec3(C, X, 0.0);
        } else if (H_seis >= 1.0 && H_seis < 2.0) {
            rgb_prime = vec3(X, C, 0.0);
        } else if (H_seis >= 2.0 && H_seis < 3.0) {
            rgb_prime = vec3(0.0, C, X);
        } else if (H_seis >= 3.0 && H_seis < 4.0) {
            rgb_prime = vec3(0.0, X, C);
        } else if (H_seis >= 4.0 && H_seis < 5.0) {
            rgb_prime = vec3(X, 0.0, C);
        } else { // H_seis >= 5.0 && H_seis < 6.0
            rgb_prime = vec3(C, 0.0, X);
        }
        float m = V - C;
        return rgb_prime + vec3(m);
    }

    void main()
    {
        // 1. Pegamos o 's' (horizontal) como Hue
        float H = tex_coord.s;

        // Variação didática: use 't' para Saturação
        //float S = tex_coord.t;
        float V = tex_coord.t;

        // 2. Pegamos os valores dos uniforms
        float S = u_saturation;
        //float V = u_value;

        // 3. Montamos o vetor e convertemos
        vec3 hsv = vec3(H, S, V);
        vec3 rgb = hsv2rgb_pedagogical(hsv);

        FragColor = vec4(rgb, 1.0);
    }
)glsl";

const GLchar *fragmentShaderSource2 = R"glsl(
    #version 400
    
    in vec2 tex_coord; // Recebido do Vertex Shader (coords [0,1])
    out vec4 FragColor;

    // Uniforms para Saturação e Valor
    uniform float u_saturation;
    uniform float u_value;

    #define TWO_PI 6.28318530718

    /*
     * Função de conversão HSV -> RGB (Pedagógica)
     * (Exatamente a mesma de antes)
     */
    vec3 hsv2rgb_pedagogical(vec3 c) {
        float H = c.x;
        float S = c.y;
        float V = c.z;
        float C = V * S;
        float H_seis = H * 6.0;
        float X = C * (1.0 - abs(mod(H_seis, 2.0) - 1.0));
        vec3 rgb_prime;

        if (H_seis >= 0.0 && H_seis < 1.0) {
            rgb_prime = vec3(C, X, 0.0);
        } else if (H_seis >= 1.0 && H_seis < 2.0) {
            rgb_prime = vec3(X, C, 0.0);
        } else if (H_seis >= 2.0 && H_seis < 3.0) {
            rgb_prime = vec3(0.0, C, X);
        } else if (H_seis >= 3.0 && H_seis < 4.0) {
            rgb_prime = vec3(0.0, X, C);
        } else if (H_seis >= 4.0 && H_seis < 5.0) {
            rgb_prime = vec3(X, 0.0, C);
        } else { // H_seis >= 5.0 && H_seis < 6.0
            rgb_prime = vec3(C, 0.0, X);
        }
        float m = V - C;
        return rgb_prime + vec3(m);
    }

    void main()
    {
        // 1. Pega a coordenada de textura [0,1]
        vec2 st = tex_coord;

        // 2. Calcula o vetor do centro (0.5) até o pixel (st)
        vec2 fromCenter = st - vec2(0.5);

        // 3. Calcula o ângulo (Hue)
        // atan(y, x) nos dá o ângulo em radianos de -PI a +PI
        float angle = atan(fromCenter.y, fromCenter.x);
        // Mapeia o ângulo [-PI, +PI] para o Hue [0, 1]
        float H = (angle / TWO_PI) + 0.5;

        // 4. Calcula o raio (Saturation)
        // length() nos dá a distância do centro
        // O raio vai de 0.0 (centro) até 0.5 (bordas do quadrado)
        float radius = length(fromCenter);
        // Multiplica por 2.0 para mapear [0.0, 0.5] para [0.0, 1.0]
        // clamp() garante que a saturação não passe de 1.0 nos cantos
        float S = clamp(radius * 2.0, 0.0, 1.0);

        // 5. Pega o Valor (Value) do uniform
        // (Poderíamos usar u_saturation também, se quiséssemos)
        float V = u_value; 

        // 6. Monta o vetor e converte
        vec3 hsv = vec3(H, S, V);
        vec3 rgb = hsv2rgb_pedagogical(hsv);

        FragColor = vec4(rgb, 1.0);
    }
)glsl";

bool keys[1024];


// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Muita atenção aqui: alguns ambientes não aceitam essas configurações
	// Você deve adaptar para a versão do OpenGL suportada por sua placa
	// Sugestão: comente essas linhas de código para desobrir a versão e
	// depois atualize (por exemplo: 4.5 com 4 e 5)
	 //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	 //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	 //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	 //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Ativa a suavização de serrilhado (MSAA) com 8 amostras por pixel
	// glfwWindowHint(GLFW_SAMPLES, 8);

	// Essencial para computadores da Apple
	// #ifdef __APPLE__
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// #endif

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola Triangulo! -- Rossana", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	// Obtendo as informações de versão
	const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte *version = glGetString(GL_VERSION);	/* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	//Inicializar o array de controle de teclas
	for (int i =0; i < 1024; i++) { keys[i] = false; }

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();
	GLuint texID = loadTexture("../assets/sprites/planta_spr.png");
	
    //Sprite spr;
    //spr.initialize(shaderID,texID,4,6,vec3(400.0,300.0,0.0),vec3(64.0*3,64.0*3,1.0));
	
	Sprite spr;
    // AQUI: Usamos nAnimations=1 e nFrames=1. texID=0 (não usado).
    spr.initialize(shaderID, 0, 1, 1, vec3(400.0, 300.0, 0.0), vec3(400.0, 400.0, 1.0)); //Tamanho maior
	//Habilitação do teste de profundidade
	
	glEnable(GL_DEPTH_TEST);
    //Habilitação da transparência
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


	glUseProgram(shaderID); // Reseta o estado do shader para evitar problemas futuros

	double prev_s = glfwGetTime();	// Define o "tempo anterior" inicial.
	double title_countdown_s = 0.1; // Intervalo para atualizar o título da janela com o FPS.

	// Criação da matriz de projeção
	mat4 projection = ortho(0.0, 800.0, 0.0, 600.0, -1.0, 1.0);

	// Utilizamos a variáveis do tipo uniform em GLSL para armazenar esse tipo de info
	// que não está nos buffers
	// Mandar a matriz de projeção para o shader
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"),1,GL_FALSE,value_ptr(projection));

	mat4 model = mat4(1); //matriz identidade

	// Ativar o primeiro buffer de textura do OpenGL
	glActiveTexture(GL_TEXTURE0);

	// Registrando o nome que o buffer da textura terá no fragment shader
	glUniform1i(glGetUniformLocation(shaderID, "tex_buffer"), 0);

	// Pegando a localização dos novos uniforms
    GLint sat_location = glGetUniformLocation(shaderID, "u_saturation");
    GLint val_location = glGetUniformLocation(shaderID, "u_value");
	
	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Este trecho de código é totalmente opcional: calcula e mostra a contagem do FPS na barra de título
		{
			double curr_s = glfwGetTime();		// Obtém o tempo atual.
			double elapsed_s = curr_s - prev_s; // Calcula o tempo decorrido desde o último frame.
			prev_s = curr_s;					// Atualiza o "tempo anterior" para o próximo frame.

			// Exibe o FPS, mas não a cada frame, para evitar oscilações excessivas.
			title_countdown_s -= elapsed_s;
			if (title_countdown_s <= 0.0 && elapsed_s > 0.0)
			{
				double fps = 1.0 / elapsed_s; // Calcula o FPS com base no tempo decorrido.

				// Cria uma string e define o FPS como título da janela.
				char tmp[256];
				sprintf(tmp, "Ola Triangulo! -- Rossana\tFPS %.2lf", fps);
				glfwSetWindowTitle(window, tmp);

				title_countdown_s = 0.1; // Reinicia o temporizador para atualizar o título periodicamente.
			}
		}

		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();
		processInput(spr);

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		// Enviando os valores de S e V para o shader
        // (Pode-se usar o tempo ou input para variar isso)
        float S = 1.0;
        float V = 1.0;
        glUniform1f(sat_location, S);
        glUniform1f(val_location, V);

        //Atualizar e desenhar o sprite
        spr.update();
        spr.draw();

		glBindVertexArray(0); // Desnecessário aqui, pois não há múltiplos VAOs

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	//glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (action == GLFW_PRESS)
	{
		keys[key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		keys[key] = false;
	}
}

// Esta função está bastante hardcoded - objetivo é compilar e "buildar" um programa de
//  shader simples e único neste exemplo de código
//  O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
//  fragmentShader source no iniçio deste arquivo
//  A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource2, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

GLuint loadTexture(string filePath)
{
	GLuint texID;

	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	// Ajusta os parâmetros de wrapping e filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Carregamento dos pixels da imagem
	int width, height, nrChannels;
	unsigned char *data = stbi_load(filePath.c_str(), &width, &height,
	&nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) //jpg, bmp
 		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
			data);
 		}
 		else //png
 		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			data);
 		}
 		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	// Liberar o data e desconectar a textura
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

void processInput(Sprite &spr)
{
	if (keys[GLFW_KEY_D])
	{
		spr.moveRight();
	}
	if (keys[GLFW_KEY_A])
	{
		spr.moveLeft();
	}
}
