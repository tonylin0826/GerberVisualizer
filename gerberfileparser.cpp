#include "gerberfileparser.hpp"
#include "gerbercanvas.hpp"


struct Tool {
    char name[5];
    double h, w;
    char shape;
};


void setfill(std::string &str, int n, char *c, bool b) {
    n = n - str.length();
    if (atof(str.c_str()) < 0) {
        if (b) {
            for (int i = 0; i < n + 1; i++) str.insert(1, c);
        } else
            for (int i = 0; i < n + 1; i++) str.append(c);
    } else {
        if (b) {
            for (int i = 0; i < n; i++) {
                str.insert(0, c);
            }
        } else
            for (int i = 0; i < n; i++) str.append(c);
    }
}

void getXy(char *ary, std::string &x, std::string &y) {
    std::string str[3];
    int m;
    for (size_t i = 0; i < strlen(ary); i++) {
        if (ary[i] == 'X')
            m = 0;
        else if (ary[i] == 'Y')
            m = 1;
        else if (ary[i] == 'D')
            m = 2;
        else if ((ary[i] <= 57 && ary[i] >= 48) || ary[i] == '-')
            str[m] += ary[i];
        else
            m = 3;
    }
    x = str[0];
    y = str[1];
}

GerberFileParser::GerberFileParser() {}

GerberLayer GerberFileParser::parse(const std::string &fileContent) {
    GerberLayer layer;

    char sh, str[50] = {0};
    int tool = 0, mode, xform, c = 0, yform, j = 0;  // 0:inch 1:mm
    bool isLead = false, color, check;
    double X, Y, x1 = 0, x2 = 0, y1 = 0, y2 = 0, tx = 0, ty = 0;

    tool = 100;
    Tool *t = new Tool[tool];
    for (int i = 0; i < tool; i++) {
        t[i].h = 0;
        t[i].w = 0;
    }

    double h, w;

    for (size_t i = 0; i < fileContent.length(); i++) {
        if (fileContent[i] != '*' && fileContent[i] != '\n' && fileContent[i] != '\0' && fileContent[i] != '%') {
            str[j] = fileContent[i];
            j++;
        } else if (fileContent[i] == '*') {
            if (str[0] == 'M' && str[2] == '2') break;
            if (str[0] == 'G' && str[1] == '0') {
            }
            if (str[2] == 'I' && str[3] == 'N') mode = 0;
            if (str[2] == 'I' && str[3] == 'N') mode = 1;
            if (str[0] == 'L' && str[2] == 'D') color = 1;
            if (str[0] == 'L' && str[2] == 'C') color = 0;
            if (str[2] == 'L') {
                isLead = true;
                std::string x, y;
                getXy(str, x, y);
                xform = atoi(x.c_str());
                yform = atoi(y.c_str());
            }
            if (str[2] == 'T') {
                isLead = false;
                std::string x, y;
                getXy(str, x, y);
                xform = atoi(x.c_str());
                yform = atoi(y.c_str());
            }
            if (str[0] == 'A' && str[1] == 'D') {
                int time = 0;
                char *token = strtok(str, "ADX,");
                while (token != NULL) {
                    if (time == 0) {
                        strcpy(t[c].name, token);
                        time++;
                    } else if (time == 1) {
                        t[c].w = atof(token);
                        time++;
                    } else if (time == 2) {
                        t[c].h = atof(token);
                        time++;
                    }
                    t[c].shape = t[c].name[2];
                    token = strtok(NULL, "ADX,");
                }
                c++;
            }
            if (str[0] == 'G' && str[1] == '5') {
                char token[2] = {'0', '0'};
                token[0] = str[4];
                token[1] = str[5];
                for (int i = 0; i < tool; i++) {
                    if (strncmp(token, t[i].name, 2) == 0) {
                        h = t[i].h;
                        w = t[i].w;
                        sh = t[i].shape;
                    }
                }
            }
            if (str[0] == 'X' || str[0] == 'Y') {
                if (str[strlen(str) - 1] == '2') {
                    std::string x, y;
                    getXy(str, x, y);
                    setfill(x, xform / 10 + xform % 10, "0", isLead);
                    setfill(y, yform / 10 + yform % 10, "0", isLead);
                    x1 = atof(x.c_str()) / pow(10.0, xform % 10);
                    y1 = atof(y.c_str()) / pow(10.0, yform % 10);
                    check = true;
                    layer.calculateMinMax(x1, y1);
                }
                if (str[strlen(str) - 1] == '1') {
                    std::string x, y;
                    getXy(str, x, y);
                    setfill(x, xform / 10 + xform % 10, "0", isLead);
                    setfill(y, yform / 10 + yform % 10, "0", isLead);
                    if (check == false) {
                        x1 = tx;
                        y1 = ty;
                    }
                    x2 = atof(x.c_str()) / pow(10.0, xform % 10);
                    y2 = atof(y.c_str()) / pow(10.0, yform % 10);
                    tx = x2;
                    ty = y2;
                    layer.calculateMinMax(x2, y2);
                }
                if (str[strlen(str) - 1] == '3') {
                    std::string x, y;
                    getXy(str, x, y);
                    setfill(x, xform / 10 + xform % 10, "0", isLead);
                    setfill(y, yform / 10 + yform % 10, "0", isLead);
                    X = atof(x.c_str()) / pow(10.0, xform % 10);
                    Y = atof(y.c_str()) / pow(10.0, yform % 10);

                    layer.appendNode(GerberPad(
                        GerberPoint2D(X, Y), w, h, sh, color == 1));

                    layer.calculateMinMax(X, Y);
                }
                if (x1 != 0 && x2 != 0 && y1 != 0 && y2 != 0) {
                    layer.appendNode(GerberVector(
                        GerberPoint2D(x1, y1), GerberPoint2D(x2, y2), w, sh, color == 1));
                    x1 = 0;
                    x2 = 0;
                    y1 = 0;
                    y2 = 0;

                    layer.calculateMinMax(x1, y1);
                    layer.calculateMinMax(x2, y2);

                    check = false;
                }
            }
            for (int k = 0; k < 50; k++) str[k] = 0;
            j = 0;
        } else
            j = 0;
    }


    return layer;
}
