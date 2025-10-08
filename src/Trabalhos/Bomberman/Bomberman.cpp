#include <cstdio>
#include <iostream>
#include <string>
#include <assert.h>
#include <cstdlib>
#include <ctime>

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
#include "DestructibleBarrier.h"
#include "IndestructibleBarrier.h"
#include "Player.h"
#include "Bomb.h"
#include "Explosion.h"
#include "DropBomb.h"

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
GLuint loadTexture(string filePath);
bool checkCollision(Sprite &one, Sprite &two);
void processInput();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 600, HEIGHT = 600;
const float CELL_SIZE = 50;
const float HALF_CELL_SIZE = CELL_SIZE / 2.0f;

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
	 tex_coord = vec2(texc.s,1.0-texc.t);
 }
 )glsl";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"glsl(
 #version 400
 in vec2 tex_coord;
 out vec4 color;

uniform sampler2D tex_buffer;
uniform vec2 offsetTex;

 void main()
 {
	 color = texture(tex_buffer,tex_coord+offsetTex);
 }
 )glsl";

bool keys[1024];
bool g_space_pressed_once = false;

bool checkCollision(Sprite &player, Sprite &barrier)
{
    float playerCollisionBoxHeightFactor = 0.6f; 
    float playerCollisionBoxWidthFactor = 0.8f;

    float playerCollisionHeight = player.getDimensions().y * playerCollisionBoxHeightFactor;
    float playerCollisionWidth = player.getDimensions().x * playerCollisionBoxWidthFactor;

    float player_x_min = player.getPos().x - playerCollisionWidth / 2.0f;
    float player_x_max = player.getPos().x + playerCollisionWidth / 2.0f;
    float player_y_min = player.getPos().y - player.getDimensions().y / 2.0f;
    float player_y_max = player_y_min + playerCollisionHeight;

    float barrier_x_min = barrier.getPos().x - barrier.getDimensions().x / 2.0f;
    float barrier_x_max = barrier.getPos().x + barrier.getDimensions().x / 2.0f;
    float barrier_y_min = barrier.getPos().y - barrier.getDimensions().y / 2.0f;
    float barrier_y_max = barrier.getPos().y + barrier.getDimensions().y / 2.0f;

    bool collisionX = player_x_max >= barrier_x_min && barrier_x_max >= player_x_min;
    bool collisionY = player_y_max >= barrier_y_min && barrier_y_max >= player_y_min;

    return collisionX && collisionY;
}

void initializeBorder(Player &player, GLuint barrierTexID, vector<IndestructibleBarrier> &barriers) {
    int cols = (HEIGHT / CELL_SIZE);
    int rows = (WIDTH / CELL_SIZE);
    float halfCell = (CELL_SIZE / 2);

    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            if (i == 0 || i == cols - 1 || j == 0 || j == cols - 1) {
                IndestructibleBarrier barrier;

                float posX = (CELL_SIZE * i) + halfCell;
                float posY = (CELL_SIZE * j) + halfCell;

                vec3 position(posX, posY, -0.2); 

                barrier.initialize(CELL_SIZE, player.getSprite().shaderID, barrierTexID, position);
                if (checkCollision(player.getSprite(), barrier)) {
                    continue;
                }
                barriers.push_back(barrier);
            }
        }
    }
}

void initializeGridOccupied(Player& player, vector<IndestructibleBarrier>& indestructibleBarriers, vector<vector<bool>>& gridOccupied)
{
    for (auto& barrier : indestructibleBarriers) {
        int gridX = barrier.getPos().x / CELL_SIZE;
        int gridY = barrier.getPos().y / CELL_SIZE;
        if (gridX < gridOccupied[0].size() && gridY < gridOccupied.size()) {
            gridOccupied[gridY][gridX] = true;
        }
    }
    int playerGridX = player.getSprite().getPos().x / CELL_SIZE;
    int playerGridY = player.getSprite().getPos().y / CELL_SIZE;

    gridOccupied[playerGridY][playerGridX] = true;      // Posição atual
    gridOccupied[playerGridY+1][playerGridX] = true;    // Uma acima
    gridOccupied[playerGridY+1][playerGridX+1] = true;  // Uma acima e a direita
    gridOccupied[playerGridY][playerGridX+1] = true;    // Uma a direita
    gridOccupied[playerGridY-1][playerGridX+1] = true;  // Uma abaixo e a direita
    gridOccupied[playerGridY-1][playerGridX] = true;    // Uma abaixo
    gridOccupied[playerGridY-1][playerGridX-1] = true;  // Uma abaixo e a esquerda
    gridOccupied[playerGridY][playerGridX-1] = true;    // Uma a esquerda
    gridOccupied[playerGridY+1][playerGridX-1] = true;  // Uma acima e a esquerda
}

void handleBombPlacement(Player& player, vector<Bomb>& bombs, GLuint bombTexID)
{
    if (g_space_pressed_once && player.hasBomb())
    {
        Sprite& playerSprite = player.getSprite();
        float bombX = player.getSprite().getPos().x;
        float bombY = player.getSprite().getPos().y;

        Bomb newBomb;
        vec3 bombPos(bombX, bombY, -0.1);
        newBomb.initialize(playerSprite.shaderID, bombTexID, bombPos);
        
        player.placeBomb();
        bombs.push_back(newBomb);
        g_space_pressed_once = false;
    }
}

void createExplosion(vec3 pos, int range, vector<Explosion>& explosions,
                     vector<IndestructibleBarrier>& indestructibleBarriers, 
                     vector<DestructibleBarrier>& destructibleBarriers,
                     vector<DropBomb>& dropBombs,
                     GLuint explosionTexID, 
                     GLuint dropBombTexID, 
                     GLuint shaderID)
{
    Explosion center;
    center.initialize(shaderID, explosionTexID, 2, 5, pos, vec3(CELL_SIZE), 2);
    explosions.push_back(center);

    vec3 directions[] = { vec3(1, 0, 0), vec3(-1, 0, 0), vec3(0, 1, 0), vec3(0, -1, 0) };
    bool hasDropped = false;

    for (const auto& dir : directions) {
        for (int i = 1; i <= range; ++i) {
            vec3 partPos = pos + dir * (float)i * CELL_SIZE;
            bool stopRay = false;

            Sprite explosionPart;
            explosionPart.initialize(shaderID, explosionTexID, 2, 5, partPos, vec3(CELL_SIZE));

            for (auto& barrier : indestructibleBarriers) {
                if (checkCollision(explosionPart, barrier)) {
                    stopRay = true;
                    break;
                }
            }
            if (stopRay) break;

            for (int j = destructibleBarriers.size() - 1; j >= 0; --j) {
                if (checkCollision(explosionPart, destructibleBarriers[j])) {
                    destructibleBarriers.erase(destructibleBarriers.begin() + j);
                    stopRay = true;

                    // Chance de 50% de dropar um item
                    if (!hasDropped && (rand() % 10) < 5) {
                        DropBomb dropBomb;
                        float dropBombScale = 0.8;
                        dropBomb.initialize(shaderID, dropBombTexID, 1, 4, partPos, vec3(CELL_SIZE * dropBombScale, CELL_SIZE * dropBombScale, 0.5));
                        dropBombs.push_back(dropBomb);
                        hasDropped = true;
                    }
                    break;
                }
            }
            
            Explosion newExplosionPart;
            newExplosionPart.initialize(shaderID, explosionTexID, 2, 5, partPos, vec3(CELL_SIZE), 2);
            explosions.push_back(newExplosionPart);

            if (stopRay) break;
        }
    }
}

void initializeDestructableBlocks(
        GLuint shaderID,
        GLuint brickTexID,
        vector<DestructibleBarrier>& destructibleBarriers,
        vector<vector<bool>>& gridOccupied)
{
    for (int y = 0; y < gridOccupied.size(); y++) {
        for (int x = 0; x < gridOccupied[0].size(); x++) {
            if (!gridOccupied[y][x]) {
                // Chance de 50% de colocar um tijolo em um espaço vazio
                if ((rand() % 10) < 5) {
                    DestructibleBarrier brick;
                    float posX = x * CELL_SIZE + HALF_CELL_SIZE;
                    float posY = y * CELL_SIZE + HALF_CELL_SIZE;
                    brick.initialize(CELL_SIZE, shaderID, brickTexID, vec3(posX, posY, -0.2));
                    destructibleBarriers.push_back(brick);
                }
            }
        }
    }
}

void resetGame(Player &player, 
        vector<Bomb>& bombs,
        vector<Explosion>& explosions,
        vector<IndestructibleBarrier>& indestructibleBarriers, 
        vector<DestructibleBarrier>& destructibleBarriers,
        vector<DropBomb>& dropBombs,
        vector<vector<bool>>& gridOccupied,
        GLuint shaderID,
        GLuint brickTexID,
        GLuint barrierTexId)
{
    bombs.clear();
    explosions.clear();
    indestructibleBarriers.clear();
    destructibleBarriers.clear();
    dropBombs.clear();
    player.reset();
    for (auto& row: gridOccupied) {
        fill(row.begin(), row.end(), false);
    }

    initializeBorder(player, barrierTexId, indestructibleBarriers);
    initializeGridOccupied(player, indestructibleBarriers, gridOccupied);
    initializeDestructableBlocks(shaderID, brickTexID, destructibleBarriers, gridOccupied);
}

int main()
{
	glfwInit();
    srand(time(NULL)); 

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Bomberman", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte *version = glGetString(GL_VERSION);	/* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	for (int i =0; i < 1024; i++) { keys[i] = false; }

	GLuint shaderID = setupShader();
	GLuint playerTexID = loadTexture("../assets/sprites/bomberman_spr.png");
	GLuint grassTexID = loadTexture("../assets/tex/grass.jpg");
	GLuint brickTexID = loadTexture("../assets/tex/brick.png");
	GLuint barrierTexID = loadTexture("../assets/tex/barrier.png");
    GLuint bombTexID = loadTexture("../assets/sprites/bomb_spr.png");
    GLuint explosionTexID = loadTexture("../assets/sprites/explosion_spr.png");
    GLuint dropBombTexID = loadTexture("../assets/sprites/drop_bomb.png");
	
    Player player;
    player.initialize(CELL_SIZE, shaderID, playerTexID, vec3(WIDTH/2, HEIGHT/2, 0.0));

    Sprite grass;
    grass.initialize(shaderID, grassTexID, 1, 1, vec3(400.0, 300.0, -0.5), vec3(WIDTH*1.5, HEIGHT*1.5, 1.0));

    vector<IndestructibleBarrier> indestructibleBarriers;
    initializeBorder(player, barrierTexID, indestructibleBarriers);

    vector<DestructibleBarrier> destructibleBarriers;
    vector<vector<bool>> gridOccupied(HEIGHT / CELL_SIZE, vector<bool>(WIDTH / CELL_SIZE, false));
    initializeGridOccupied(player, indestructibleBarriers, gridOccupied);
    initializeDestructableBlocks(shaderID, brickTexID, destructibleBarriers, gridOccupied);

    vector<Bomb> bombs;
    vector<Explosion> explosions; 
    vector<DropBomb> dropBombs; 

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(shaderID); 

	double prev_s = glfwGetTime();	
	double title_countdown_s = 0.1; 

	mat4 projection = ortho(0.0, 600.0, 0.0, 600.0, -1.0, 1.0);

	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"),1,GL_FALSE,value_ptr(projection));

	mat4 model = mat4(1); 

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shaderID, "tex_buffer"), 0);
	
	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);


        if (player.isAlive()) {
            player.handleInput(keys);

            for (Sprite explosion : explosions)
            {
                if (checkCollision(player.getSprite(), explosion))
                {
                    player.die();
                    break;
                }
            }
            for (int i = dropBombs.size() - 1; i >= 0; i--)
            {
                if (checkCollision(player.getSprite(), dropBombs[i]))
                {
                    player.increaseBombs();
                    dropBombs.erase(dropBombs.begin() + i);
                }
            }
        } else {
            resetGame(player, bombs, explosions, indestructibleBarriers, 
                    destructibleBarriers, dropBombs, gridOccupied, 
                    shaderID, brickTexID, barrierTexID);
        }

        handleBombPlacement(player, bombs, bombTexID);

        for (auto& bomb : bombs)
        {
            bomb.update();
        }

        for (int i = bombs.size() - 1; i >= 0; i--)
        {
            if (bombs[i].getState() == BombState::EXPLODED)
            {
                player.restoreBomb();
                vec3 bombPos = bombs[i].getSprite().getPos();
                createExplosion(bombPos, 2, explosions, 
                        indestructibleBarriers, destructibleBarriers, dropBombs, 
                        explosionTexID, dropBombTexID, shaderID);
                bombs.erase(bombs.begin() + i);
            }
        }

        for (int i = explosions.size() - 1; i >= 0; i--)
        {
            if (explosions[i].hasExpired())
            {
                explosions.erase(explosions.begin() + i);
            }
        }

        grass.update();
        grass.draw();

        for (auto &barrier : destructibleBarriers) {
            barrier.update();
            barrier.draw();
        }

        for (auto &barrier : indestructibleBarriers) {
            barrier.update();
            barrier.draw();
        }

        for (auto &dropBomb: dropBombs)
        {
            dropBomb.update();
            dropBomb.draw();
        }

        for (auto &bomb : bombs)
        {
            bomb.draw();
        }

        for (auto &explosion : explosions)
        {
            explosion.update();
            explosion.draw();
        }

        player.update(indestructibleBarriers, destructibleBarriers);
        player.draw();

		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

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

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        g_space_pressed_once = true;
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
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else //png
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
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
