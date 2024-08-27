#include <iostream>
#include <string>
#include <fstream>
#include <locale>
#include <codecvt>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"


struct charFilter {
    bool tl, tr, bl, br;
    std::wstring character;

    charFilter(bool tl, bool tr, bool bl, bool br, const std::wstring& character)
        : tl(tl), tr(tr), bl(bl), br(br), character(character)
    {
    }
    charFilter() = default;
};

charFilter filters[16];

void initCharFilters() {
    filters[0] = charFilter(false, false, false, false, L" "); //special case for all black
    filters[1] = charFilter(false, false, false, true, L"▗");
    filters[2] = charFilter(false, false, true, false, L"▖");
    filters[3] = charFilter(false, false, true, true, L"▄");
    filters[4] = charFilter(false, true, false, false, L"▝");
    filters[5] = charFilter(false, true, false, true, L"▐");
    filters[6] = charFilter(false, true, true, false, L"▞");
    filters[7] = charFilter(false, true, true, true, L"▟");
    filters[8] = charFilter(true, false, false, false, L"▘");
    filters[9] = charFilter(true, false, false, true, L"▚");
    filters[10] = charFilter(true, false, true, false, L"▌");
    filters[11] = charFilter(true, false, true, true, L"▙");
    filters[12] = charFilter(true, true, false, false, L"▀");
    filters[13] = charFilter(true, true, false, true, L"▜");
    filters[14] = charFilter(true, true, true, false, L"▛");
    filters[15] = charFilter(true, true, true, true, L"█");
}

std::wstring getCorrectChar(bool tl, bool tr, bool bl, bool br) {
    for (int i = 0; i < 16; i++) {
        if (
            ((filters[i].tl == tl) && (filters[i].tr == tr)) &&
            ((filters[i].bl == bl) && (filters[i].br == br))) {
            return filters[i].character;
        }
    }
}

//img-to-char.exe [filename] [bw threshold] [scaling multp]

int main(int argc, char* argv[]) {
    if (argc != 4) {
        //tell user too many/too little arguments
        std::cout << "wrong arguments\n"; //just kidding
        return 1;
    }


    std::cout << argv[1] << std::endl;
    int bwthresh = std::stoi(argv[2]);
    float scale = std::stof(argv[3]);

    int imageWidth, imageHeight, components;
    unsigned char* imgBitmap = stbi_load(argv[1], &imageWidth, &imageHeight, &components, 1);
    if (stbi_failure_reason()) {
        std::cout << stbi_failure_reason() << std::endl;
        return 1;
    }

    std::cout << std::to_string(imageWidth) << " " << std::to_string(imageHeight) << " " << std::to_string(components) << " ";

    unsigned char* localImgBuffer = new unsigned char[imageWidth * imageHeight];
    std::memcpy(localImgBuffer, imgBitmap, imageWidth * imageHeight);
    stbi_image_free(imgBitmap);

    initCharFilters();

    unsigned char* convertedBuf;
    int convertedImgWidth, convertedImgHeight;
    if (scale != 1.0) {
        //do scaling
        //some normal nearest neighbor interpolation
        convertedImgWidth = (float)imageWidth * scale;
        convertedImgHeight = (float)imageHeight * scale;

        convertedBuf = new unsigned char[convertedImgHeight * convertedImgWidth];

        //use floats
        //i/convertedwidth * imgwidth = x
        for (int i = 0; i < convertedImgHeight; i++) {
            int y = ((float)i/convertedImgHeight) * imageHeight;
            for (int j = 0; j < convertedImgWidth; j++) {
                int x = ((float)j / convertedImgWidth) * imageWidth;
                convertedBuf[(i * convertedImgWidth) + j] = localImgBuffer[(y * imageWidth) + x];
            }
        }

    }
    else {
        convertedBuf = localImgBuffer;
        convertedImgWidth = imageWidth;
        convertedImgHeight = imageHeight;
    }

    std::wstring charbuf;
    for (int i = 0; i < convertedImgHeight; i += 2) {
        for (int j = 0; j < convertedImgWidth; j += 2) {
            bool
                tl = convertedBuf[(i * convertedImgWidth) + j] > bwthresh,
                tr = j + 1 < convertedImgWidth ? convertedBuf[(i * convertedImgWidth) + j + 1] > bwthresh : false,
                bl = i + 1 < convertedImgHeight ? convertedBuf[((i + 1) * convertedImgWidth) + j] > bwthresh : false,
                br = (j + 1 < convertedImgWidth) && (i + 1 < convertedImgHeight) ? convertedBuf[((i + 1) * convertedImgWidth + j + 1)] > bwthresh : false;

            charbuf += getCorrectChar(tl, tr, bl, br);
        }
        charbuf += L"\n";
    }

    const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());
    std::wofstream outpfile(std::string(argv[1]) + std::string(".txt"), std::ios::binary);
    outpfile.imbue(utf8_locale);

    if (!outpfile) {
        std::cout << "error open file for writing \n";
        return 1;
    }

    outpfile << charbuf;

    if (!outpfile) {
        std::cout << "error writing to file" << std::endl;
        return 1;
    }

    outpfile.close();
    return 0;
} 