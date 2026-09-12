#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ctime>
#include <cstdlib>
#include <vector>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define CGRIDX 50
#define CGRIDY 75
#define SPACINGNSIZE 50

#define WIDTH 20
#define HEIGHT 20
#define MWIDTH WIDTH-1
#define MHEIGHT HEIGHT-1
#define AWIDTH WIDTH+1
#define AHEIGHT HEIGHT+1

#define MINE_COUNT 50
// just in case
#if MINE_COUNT >= WIDTH*HEIGHT
#define MINE_COUNT WIDTH*HEIGHT-1
#endif
#if MINE_COUNT <= 0
#define MINE_COUNT 1
#endif

#define SCREENW 600
#define SCREENH 600

#define TEXTX -0.5f
#define TEXTY 0.9f
#define TEXTS 0.1f
#define FACEX 0.5f
#define FACEY 0.85f
#define FACES 0.15f



float cswidth = SCREENW;
float csheight = SCREENH;

#define MOUSEISOVERTILE (((msx) > (x*SPACINGNSIZE/2+CGRIDX)* (cswidth/SCREENW)) && ((msx) < (x*SPACINGNSIZE/2+SPACINGNSIZE/2+CGRIDX)* (cswidth/SCREENW)) && ((msy) > (y*SPACINGNSIZE/2+CGRIDY)*(csheight/SCREENH)) && ((msy) < (y*SPACINGNSIZE/2+SPACINGNSIZE/2+CGRIDY)*(csheight/SCREENH)))

using namespace std; 

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    cswidth = width;
    csheight = height;
    glViewport(0, 0, width, height);
}

double msx, msy;
int rmstate, prm, lmstate, plm;

bool loseState = false;

// Store map
short minesPlaced = 0;
uint8_t map[WIDTH][HEIGHT];
uint8_t c_map[WIDTH][HEIGHT];

short mineCount(short &x, short &y /*captured by reference to save memory*/){
    short nc = 0;
    // nextdoor ~
    if(x>0)if(map[x-1][y] == 9)nc++;
    if(x<MWIDTH)if(map[x+1][y] == 9)nc++;
    // top ~
    if(x>0&&y>0)if(map[x-1][y-1] == 9)nc++;
    if(y>0)if(map[x][y-1] == 9)nc++;
    if(x<MWIDTH&&y>0)if(map[x+1][y-1] == 9)nc++;
    // bottom ~
    if(x>0&&y<MHEIGHT)if(map[x-1][y+1] == 9)nc++;
    if(y<MHEIGHT)if(map[x][y+1] == 9)nc++;
    if(x<MWIDTH&&y<MHEIGHT)if(map[x+1][y+1] == 9)nc++;

    return nc;
}

class tile{
    public:
    static uint32_t idcount;
    short x = -1;
    short y = -1;
    uint32_t id = 0;
    tile(){
        
        idcount++;
    }
    void initId(){
        id = (y*WIDTH) + x;
    }
};
uint32_t tile::idcount = 0;

vector<tile> alrAcc = {};
void zeroTomfoolery(tile (*tiles)[8], short x, short y /*captured by reference to save memory*/){
    // nextdoor ~
    if(x>0)if(map[x-1][y] != 9){(*tiles)[0].x=x-1;(*tiles)[0].y=y;}
    if(x<MWIDTH)if(map[x+1][y] != 9){(*tiles)[1].x=x+1;(*tiles)[1].y=y;}
    // top ~
    if(x>0&&y>0)if(map[x-1][y-1] != 9){(*tiles)[2].x=x-1;(*tiles)[2].y=y-1;}
    if(y>0)if(map[x][y-1] != 9){(*tiles)[3].x=x;(*tiles)[3].y=y-1;}
    if(x<MWIDTH&&y>0)if(map[x+1][y-1] != 9){(*tiles)[4].x=x+1;(*tiles)[4].y=y-1;}
    // bottom ~
    if(x>0&&y<MHEIGHT)if(map[x-1][y+1] != 9){(*tiles)[5].x=x-1;(*tiles)[5].y=y+1;}
    if(y<MHEIGHT)if(map[x][y+1] != 9){(*tiles)[6].x=x;(*tiles)[6].y=y+1;}
    if(x<MWIDTH&&y<MHEIGHT)if(map[x+1][y+1] != 9){(*tiles)[7].x=x+1;(*tiles)[7].y=y+1;}
    
}

void setupmap(){
    // Place mines
    short ry = 0;
    short rx = 0;
    while(minesPlaced < MINE_COUNT && minesPlaced < WIDTH*HEIGHT){
        ry = rand()%HEIGHT;
        rx = rand()%WIDTH;
        if(map[rx][ry] != 9){
            map[rx][ry] = 9;
            minesPlaced++;
        }
        /* old code
        for(short y = 0; y < HEIGHT; y++){
            for(short x = 0; x < WIDTH; x++){
                if(map[x][y] == 9)continue; // skip already placed mines
                if(rand()%16 == 1 && minesPlaced < MINE_COUNT){
                    minesPlaced++;
                    map[x][y] = 9;
                }else map[x][y] = 0;
            }
        }*/
    }
    // Update tiles to have their corresponding #'s
    for(short y = 0; y < HEIGHT; y++){
        for(short x = 0; x < WIDTH; x++){
            if(map[x][y] == 9)continue; // again skip
            else map[x][y] = mineCount(x, y);
        }
    }
}
// GLSL; pertty small so I didn't put them into separate files
const char* vshade = R"(
    
#version 330
layout (location = 0) in vec3 apos;
layout (location = 1) in vec2 atcoords;

out vec2 tcoords;

void main()
{
    gl_Position = vec4(apos, 1.0);
    tcoords = atcoords;
}
)";
const char* _vshade = R"(
    
#version 330
layout (location = 0) in vec2 tpos;

void main()
{
    gl_Position = vec4(tpos, 0.0, 1.0);
}
)";
const char* _fshade = R"(
#version 330

out vec4 col;

void main(){
    col = vec4(0.85,0.85,0.85,1.0);
}
)";
const char* fshade = R"(

#version 330 core
out vec4 fc;
  
in vec2 tcoords;

uniform sampler2D texture;

void main()
{
    fc = texture(texture, tcoords);
}

)";
//outline
float* vertices = new float[]{
    // position          // coordinates of texture
     0.5f,  0.5f, 0.0f,   0.1f, 1.0f,   // top right
     0.5f, -0.5f, 0.0f,   0.1f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   0.0f, 1.0f    // top left 
};
float vertiA[30*WIDTH*HEIGHT];
unsigned int indices[] = {  
        0, 1, 3, 
        1, 2, 3  
};
float verticesN[120];
void retVeArr(int index){
    int j = 0;
    for(int i = index; i < index+30; i++){
        vertiA[i] = vertices[j];
        j++;
    }
}
void chZero(short x, short y){
    tile n;
    n.x = x;
    n.y = y;
    n.initId();
    for(tile j:alrAcc)if(j.id==n.id)return;
    alrAcc.push_back(n);
    tile t[8];
    zeroTomfoolery(&t, x, y);
    if(map[x][y]!=0)return;
    for(tile i:t){
        
        if(i.x < 0 || i.y < 0 || c_map[i.x][i.y]==1)continue;
        c_map[i.x][i.y] = 2;
        chZero(i.x,i.y);
    }
}
void adjVerticies(float x, float y, float mul, float width=SPACINGNSIZE, float height=SPACINGNSIZE){
    if(vertices != nullptr)delete vertices;
    
    float xx = static_cast<float>(x-SCREENW)/SCREENW;
    float yy = static_cast<float>(y+height)/SCREENH;
    float xw = static_cast<float>(width)/SCREENW;
    float yh = static_cast<float>(height)/SCREENH;
    vertices = new float[]{
        // position          // coordinates of texture
        xx+xw, yy, 0.0f,   mul/12.0f, 1.0f,   // top right
        xx+xw, yy+yh, 0.0f,   mul/12.0f, 0.0f,   // bottom right
        xx, yy+yh, 0.0f,   (mul/12.0f)-(1.0f/12.0f), 0.0f,   // bottom left

        xx+xw, yy, 0.0f,   mul/12.0f, 1.0f,   // top right
        xx, yy+yh, 0.0f,   (mul/12.0f)-(1.0f/12.0f), 0.0f,   // bottom left
        xx,  yy, 0.0f,   (mul/12.0f)-(1.0f/12.0f), 1.0f    // top left 
    };
}
int minesl = MINE_COUNT;
short faceState = 0;
void updBuffer4Ti(){
    int c = 0;
    for(short y = 0; y < HEIGHT; y++){
        for(short x = 0; x < WIDTH; x++){
            if(lmstate == GLFW_PRESS)faceState = 1; else faceState = 0;
            if(loseState){
                faceState = 2;
                plm = prm = GLFW_RELEASE;
                if(map[x][y] == 9)c_map[x][y] = 2;
            }
            if(lmstate == GLFW_RELEASE && plm == GLFW_PRESS && MOUSEISOVERTILE){c_map[x][y] = 2;if(map[x][y] == 9)loseState = true;} else if(c_map[x][y] != 2 && rmstate == GLFW_RELEASE && prm == GLFW_PRESS && MOUSEISOVERTILE){c_map[x][y] = c_map[x][y] == 1 ? 0 : 1;minesl+=c_map[x][y] == 1 ? -1 : 1;}
            if(c_map[x][y] == 2 && map[x][y] == 0){
                chZero(x,y);
            }
            
            adjVerticies(((x)*SPACINGNSIZE)+(CGRIDX*2),((HEIGHT-y)*SPACINGNSIZE)-(CGRIDY*2)-((HEIGHT-10)*SPACINGNSIZE),(c_map[x][y] == 2?static_cast<float>(map[x][y]):10.0+c_map[x][y]));
            retVeArr(c*30);
            
            c++;
        }
    }
}


int pow(int x, int y){
    y--;
    if(y < 0)return 1;
    return x*pow(x,y);
}
void loadNumOfMinesLeft(){
    for(short i = 0; i < 4; i++){
        float d = i>0 ? static_cast<int>(minesl/(pow(10,i))) % 10 : minesl%10;
        d++;
        if(minesl < 0)d=1;
        //tl
        verticesN[i*30] = TEXTX-(TEXTS*i);
        verticesN[i*30+1] = TEXTY;
        verticesN[i*30+2] = 0.0f;
        verticesN[i*30+3] = d*0.1f-0.1f;
        verticesN[i*30+4] = 0.5f;
        //tr
        verticesN[i*30+5] = TEXTX-(TEXTS*i)+TEXTS;
        verticesN[i*30+6] = TEXTY;
        verticesN[i*30+7] = 0.0f;
        verticesN[i*30+8] = d*0.1f;
        verticesN[i*30+9] = 0.5f;
        //bl
        verticesN[i*30+10] = TEXTX-(TEXTS*i);
        verticesN[i*30+11] = TEXTY+TEXTS;
        verticesN[i*30+12] = 0.0f;
        verticesN[i*30+13] = d*0.1f-0.1f;
        verticesN[i*30+14] = 0.0f;
        //tr
        verticesN[i*30+15] = TEXTX-(TEXTS*i)+TEXTS;
        verticesN[i*30+16] = TEXTY;
        verticesN[i*30+17] = 0.0f;
        verticesN[i*30+18] = d*0.1f;
        verticesN[i*30+19] = 0.5f;
        //bl
        verticesN[i*30+20] = TEXTX-(TEXTS*i);
        verticesN[i*30+21] = TEXTY+TEXTS;
        verticesN[i*30+22] = 0.0f;
        verticesN[i*30+23] = d*0.1f-0.1f;
        verticesN[i*30+24] = 0.0f;
        //br
        verticesN[i*30+25] = TEXTX-(TEXTS*i)+TEXTS;
        verticesN[i*30+26] = TEXTY+TEXTS;
        verticesN[i*30+27] = 0.0f;
        verticesN[i*30+28] = d*0.1f;
        verticesN[i*30+29] = 0.0f;
    }
    
}

void loadFace(){
    
    //tl
    verticesN[90] = FACEX;
    verticesN[91] = FACEY;
    verticesN[92] = 0.0f;
    verticesN[93] = faceState*0.1f;
    verticesN[94] = 1.0f;
    //tr
    verticesN[95] = FACEX+FACES;
    verticesN[96] = FACEY;
    verticesN[97] = 0.0f;
    verticesN[98] = faceState*0.1f+0.1f;
    verticesN[99] = 1.0f;
    //bl
    verticesN[100] = FACEX;
    verticesN[101] = FACEY+FACES;
    verticesN[102] = 0.0f;
    verticesN[103] = faceState*0.1f;
    verticesN[104] = 0.5f;
    //tr
    verticesN[105] = FACEX+FACES;
    verticesN[106] = FACEY;
    verticesN[107] = 0.0f;
    verticesN[108] = faceState*0.1f+0.1f;
    verticesN[109] = 1.0f;
    //bl
    verticesN[110] = FACEX;
    verticesN[111] = FACEY+FACES;
    verticesN[112] = 0.0f;
    verticesN[113] = faceState*0.1f;
    verticesN[114] = 0.5f;
    //br
    verticesN[115] = FACEX+FACES;
    verticesN[116] = FACEY+FACES;
    verticesN[117] = 0.0f;
    verticesN[118] = faceState*0.1f+0.1f;
    verticesN[119] = 0.5f;
}
unsigned int frame = 0;

int main()
{
    srand(time(0));
    //------------------------------------------------------------
    if (!glfwInit())
    {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    window = glfwCreateWindow(SCREENW, SCREENH, "Minesweeper", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to open GLFW window" << endl;
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    glViewport(0, 0, SCREENW, SCREENH);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //-------------------------------------------------------------
    // TOP BAR
    float BARVERT[] = {
        -1.0f, 1.0f,
        -1.0f, 0.8f,
        1.0f, 0.8f,

        1.0f, 0.8f,
        1.0f, 1.0f,
        -1.0f, 1.0f        
    };
    unsigned int MVBO, MVAO;
    glGenVertexArrays(1, &MVAO);
    glGenBuffers(1, &MVBO);//
    glBindVertexArray(MVAO);
    glBindBuffer(GL_ARRAY_BUFFER, MVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(BARVERT), BARVERT, GL_STATIC_DRAW);//
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //-------------------------------------------------------------
    unsigned int VBO, VAO, VAON;
    glGenVertexArrays(1, &VAO);
    glGenVertexArrays(1, &VAON);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertiA), vertiA, GL_STREAM_DRAW);

    // pos att
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture pos att
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Vertex Array for number fonts
    glBindVertexArray(VAON);
    

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesN), verticesN, GL_STREAM_DRAW);

    // pos att
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture pos att
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // textures
    unsigned int tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Load image atlas
    int iwid, ihei, nr;
    //stbi_set_flip_vertically_on_load(true);
    unsigned char* img = stbi_load("src/ms_atlas.png", &iwid, &ihei, &nr, 0);
    if(img){
        int format = nr == 4? GL_RGBA: GL_RGB;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, iwid, ihei, 0, format, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{cout << "Error occurred";}
    stbi_image_free(img);

    unsigned int texn;
    glGenTextures(1, &texn);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texn);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    img = stbi_load("src/num_f.png", &iwid, &ihei, &nr, 0);
    if(img){
        int format = nr == 4? GL_RGBA: GL_RGB;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, iwid, ihei, 0, format, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{cout << "Error occurred";}
    stbi_image_free(img);
    // shader
    GLuint vSh;
    vSh = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vSh, 1, &vshade, NULL);
    glCompileShader(vSh);
    int  success;

    GLuint fSh;
    fSh = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fSh, 1, &fshade, NULL);
    glCompileShader(fSh);

    GLuint shaderC = glCreateProgram();
    glAttachShader(shaderC, vSh);
    glAttachShader(shaderC, fSh);
    glLinkProgram(shaderC);

    glUseProgram(shaderC);
    glUniform1i(glGetUniformLocation(shaderC, "ourTexture"),0);
    
    glDeleteShader(vSh);
    glDeleteShader(fSh);

    ///////////
    unsigned int vesh = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vesh, 1, &_vshade, NULL);
    glCompileShader(vesh);

    unsigned int fesh = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fesh, 1, &_fshade, NULL);
    glCompileShader(fesh);

    unsigned int bar = glCreateProgram();
    glAttachShader(bar, vesh);
    glAttachShader(bar, fesh);
    glLinkProgram(bar);

    glDeleteShader(vesh);
   
    // Setup
    setupmap();
    while(!glfwWindowShouldClose(window))
    {
        glfwGetCursorPos(window, &msx, &msy);
        //std::cout << msx << ", " << msy << ") (";
        prm = rmstate;
        rmstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        plm = lmstate;
        lmstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.9f,0.9f,0.9f,1.0f);

        glUseProgram(bar);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_ARRAY_BUFFER, MVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(BARVERT), BARVERT, GL_STATIC_DRAW);
        glBindVertexArray(MVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(shaderC);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        updBuffer4Ti();
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertiA), vertiA, GL_STREAM_DRAW);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0,6 *WIDTH*HEIGHT);

        glUseProgram(shaderC);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texn);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        loadNumOfMinesLeft();
        loadFace();
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticesN), verticesN, GL_STREAM_DRAW);
        glBindVertexArray(VAON);
        glDrawArrays(GL_TRIANGLES, 0,24);

        glfwSwapBuffers(window);
        glfwPollEvents(); 
        //
        alrAcc.clear();
        tile::idcount = 0;   
    }
    
    glDeleteBuffers(1,&VAO);
    glDeleteBuffers(1,&VBO);
    delete vertices;
    glfwTerminate();
    return 0;
}