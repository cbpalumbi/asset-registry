#include "World.h"

std::vector<WorldObject> createWorld() {
    return {
        // Golden chest - center of the scene
        {{ 0,   0 },    "Rocks and Chest/Chest/GoldenChest/1.png"},
        {{ 60,  -30 },  "Rocks and Chest/Chest/GoldenChest/2.png"},

        // Iron chests nearby
        {{ -80, 40 },   "Rocks and Chest/Chest/IronChest/1.png"},
        {{ 100, 50 },   "Rocks and Chest/Chest/IronChest/2.png"},

        // Trees surrounding the chests
        {{ -120, -80 }, "Trees/Tree1.png"},
        {{ 80,  -100 }, "Trees/Tree2.png"},
        {{ -60, -120 }, "Trees/Tree3.png"},
        {{ 140, -60 },  "Trees/Tree1.png"},
        {{ -140, 60 },  "Trees/Tree2.png"},
        {{ 120,  100 }, "Trees/Tree3.png"},
        {{ -100, 120 }, "Trees/Tree1.png"},
        {{ 60,   130 }, "Trees/Branch.png"},
        {{ -30,  140 }, "Trees/Branch.png"},

        // Rocks scattered close by
        {{ -60,  60 },  "Rocks and Chest/Rocks/Rock1.png"},
        {{ 80,   20 },  "Rocks and Chest/Rocks/Rock2.png"},
        {{ -80, -40 },  "Rocks and Chest/Rocks/Rock3.png"},
        {{ 40,   80 },  "Rocks and Chest/Rocks/Rock4.png"},
        {{ -40, -80 },  "Rocks and Chest/Rocks/Rock5.png"},
        {{ 120,  -20 }, "Rocks and Chest/Rocks/Rock6.png"},

        // Magic stones in a tight arc around the chest
        {{ -160, 0 },   "Rocks and Chest/MagicStoons/MagicStones1.png"},
        {{ -120, -100 },"Rocks and Chest/MagicStoons/MagicStones2.png"},
        {{ 0,   -150 }, "Rocks and Chest/MagicStoons/MagicStones3.png"},
        {{ 120, -100 }, "Rocks and Chest/MagicStoons/MagicStones4.png"},
        {{ 160,  0 },   "Rocks and Chest/MagicStoons/MagicStones5.png"},
        {{ 100,  130 }, "Rocks and Chest/MagicStoons/MagicStones6.png"},

        // Lamp and sign as entrance markers
        {{ -20,  170 }, "Wooden/Lamp.png"},
        {{ 30,   170 }, "Wooden/Sign.PNG"},
    };
}