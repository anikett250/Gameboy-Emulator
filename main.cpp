#include <iostream>
#include <array>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

class Gameboy
{
public:
    // --- 1. CPU Registers (Sharp LR35902) ---
    struct
    {
        uint8_t a = 0x01, f = 0xB0;
        uint8_t b = 0x00, c = 0x13;
        uint8_t d = 0x00, e = 0xD8;
        uint8_t h = 0x01, l = 0x4D;
        uint16_t sp = 0xFFFE;
        uint16_t pc = 0x0100;
        bool ime = false;
    } reg;

    uint8_t opcode{};

    // --- 2. Segmented Memory Map (64 KB Address Space) ---
    std::vector<uint8_t> rom;
    std::array<uint8_t, 0x2000> vram{};
    std::array<uint8_t, 0x2000> eram{};
    std::array<uint8_t, 0x2000> wram{};
    std::array<uint8_t, 0x00A0> oam{};
    std::array<uint8_t, 0x0080> io{};
    std::array<uint8_t, 0x007F> hram{};
    uint8_t ie{};

    // --- 3. Input & Display (160x144 instead of CHIP-8's 64x32) ---
    uint8_t joypad = 0xFF;
    std::array<uint32_t, 160 * 144> video{};

    // --- 4. Core Bus & Execution Methods ---
    bool loadROM(const std::string &filepath);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
    int step();
};

uint8_t Gameboy::read(uint16_t addr)
{
    if (addr <= 0x3FFF)
        return rom[addr];
    else if (addr <= 0x7FFF)
        return rom[addr];
    else if (addr <= 0x9FFF)
        return vram[addr - 0x8000];
    else if (addr <= 0xBFFF)
        return eram[addr - 0xA000];
    else if (addr <= 0xDFFF)
        return wram[addr - 0xC000];
    else if (addr <= 0xFDFF)
        return wram[addr - 0xE000];
    else if (addr <= 0xFE9F)
        return oam[addr - 0xFE00];
    else if (addr <= 0xFEFF)
        return 0xFF;
    else if (addr <= 0xFF7F)
        return io[addr - 0xFF00];
    else if (addr <= 0xFFFE)
        return hram[addr - 0xFF80];
    else
        return ie;
}

void Gameboy::write(uint16_t addr, uint8_t val)
{
    if (addr <= 0x7FFF)
        return;
    else if (addr <= 0x9FFF)
        vram[addr - 0x8000] = val;
    else if (addr <= 0xBFFF)
        eram[addr - 0xA000] = val;
    else if (addr <= 0xDFFF)
        wram[addr - 0xC000] = val;
    else if (addr <= 0xFDFF)
        wram[addr - 0xE000] = val;
    else if (addr <= 0xFE9F)
        oam[addr - 0xFE00] = val;
    else if (addr <= 0xFEFF)
        return;
    else if (addr <= 0xFF7F)
        io[addr - 0xFF00] = val;
    else if (addr <= 0xFFFE)
        hram[addr - 0xFF80] = val;
    else
        ie = val;
}

bool Gameboy::loadROM(const std::string &filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return false;

    std::streamsize size = file.tellg();
    if (size <= 0)
        return false;

    rom.resize(size);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(rom.data()), size);

    return true;
}

int main()
{
    Gameboy gameboy;
    if (!gameboy.loadROM("Pokemon - Red Version.gb")){
        std::cerr << "Failed to Load ROM\n";
        return 1;
    }

    sf::RenderWindow window(
        sf::VideoMode({1280, 720}),
        "Gameboy Emulator");
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        // SFML 3 event polling syntax
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        window.clear(sf::Color::Black);
        window.display();
    }
}