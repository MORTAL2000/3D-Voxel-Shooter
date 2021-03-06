

#include "Ammo.h"

void Ammo::render() {
    ShaderLoader::useProgramm(PROGRAMM);
    glDisable(GL_BLEND);
    glBindVertexArray(VAO);
    ShaderLoader::setUniform(PROGRAMM,StateEssentials::get().camera.GetViewMatrix(),"view");
    ShaderLoader::setUniform(PROGRAMM,StateEssentials::get().camera.Position,"cameraPos");
    ShaderLoader::setUniform(PROGRAMM,StateEssentials::get().windowManager.perspectiveProjection,"projection");
    ShaderLoader::setUniform(PROGRAMM,model,"model");
    glDrawArrays(GL_POINTS, 0, Graphicsbuffer.size()/2);
    glBindVertexArray(0);
    glEnable(GL_BLEND);
}

void Ammo::update(float &elapsed) {
    model = glm::rotate(model,0.06f,glm::vec3(0.f,1.f,0.f));
    bool buffer_cleared =false;
    for(auto& pkg:packages)
    {

        pkg.update(elapsed,need_refactor);
        if(!buffer_cleared&&need_refactor)
        {
            Graphicsbuffer.clear();
            buffer_cleared= true;
        }
        if(need_refactor&&pkg.respawnTimer<0.01
        )
        {
            Graphicsbuffer.emplace_back(pkg.position);
            Graphicsbuffer.emplace_back(pkg.color);
        }
    }
    if(need_refactor)
    {
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glBufferSubData(GL_ARRAY_BUFFER,0, Graphicsbuffer.size()* sizeof(glm::vec3) , Graphicsbuffer.data());
        need_refactor = false;
    }
}

void Ammo::create(Terrain &ter, int amount, float respawntime) {
    int found_places = amount;
    int max_tries = 10*amount;
    auto terrain = ter.getTerrainData();
    while(found_places>0 && max_tries>0)
    {
        int x = rand()%ter.getSize();
        int z = rand()%ter.getSize();
        if(ter.isTerrain(x,z))
        {
            int y = ter.getY(x,z);
            BLOCK_TYPE buf = ter.getBlocktype({x,y,z});
            y++;
            AmmoPackage buffer;
            buffer.position=glm::vec3(x,y,z);

            int randomChance = rand()%100;

            if(randomChance<30)
            {
                //heavy
                buffer.color={0.058, 1, 0.447};
                buffer.m_type = AmmoType::heavy;
            }
            if(randomChance>=30&&randomChance<=80)
            {
                //light
                buffer.color={1, 0.317, 0.058};
                buffer.m_type = AmmoType::light;
            }
            if(randomChance>70)
            {
                //sniper
                buffer.m_type = AmmoType::sniper;
                buffer.color={0.058, 0.074, 1};
            }

            bool alreadyplaced = false;
            for(auto& ber:packages)
            {
                if(ber.position.x==x && ber.position.z == z)
                {
                    alreadyplaced=true;
                }
            }
            if(!alreadyplaced)
            {
                packages.emplace_back(buffer);
                found_places--;
            }
        }
        max_tries--;
    }

    for(auto& graphics_data:packages)
    {
        Graphicsbuffer.emplace_back(graphics_data.position);
        Graphicsbuffer.emplace_back(graphics_data.color);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,Graphicsbuffer.size() * sizeof(glm::vec3), Graphicsbuffer.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3), (void*)sizeof(glm::vec3));
}

Ammo::Ammo() {
    PROGRAMM=ShaderLoader::createProgram({{"berry.frag"},{"berry.geom"},{"berry.vert"}});
    glGenVertexArrays(1,&VAO);
    std::cout << "Ammo vao:" << VAO<< std::endl;
    glGenBuffers(1,&VBO);

    std::string path_to_shader_dir="../data/";
    pickupsound.loadFromFile(path_to_shader_dir+"sounds/pickup_ammo.wav");

    model = glm::mat4(1.f);
}

void Ammo::playPickup() {
    sound.setBuffer(pickupsound);
    sound.play();
}


