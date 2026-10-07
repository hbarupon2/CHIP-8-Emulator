/**
 * Assembler for the CHIP-8 instruction set.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    NONE,
    V0_ADDR,
    I_ADDR,
    ADDR,
    B_VX,
    DT_VX,
    F_VX,
    I_VX,
    ST_VX,
    VX_BYTE,
    VX_DT,
    VX_I,
    VX_K,
    VX_VY,
    VX_VY_N,
    VX,
} Shape;

typedef struct {
    const char *mnemonic;
    Shape shape;
    uint16_t base;
} AsmOp;

static const AsmOp ops[] = {
    {"CLS", NONE, 0x00E0},
    {"RET", NONE, 0x00EE},
    {"JP", ADDR, 0x1000},
    {"CALL", ADDR, 0x2000},
    {"SE", VX_BYTE, 0x3000},
    {"SNE", VX_BYTE, 0x4000},
    {"SE", VX_VY, 0x5000},
    {"LD", VX_BYTE, 0x6000},
    {"ADD", VX_BYTE, 0x7000},
    {"LD", VX_VY, 0x8000},
    {"OR", VX_VY, 0x8001},
    {"AND", VX_VY, 0x8002},
    {"XOR", VX_VY, 0x8003},
    {"ADD", VX_VY, 0x8004},
    {"SUB", VX_VY, 0x8005},
    {"SHR", VX, 0x8006},
    {"SUBN", VX_VY, 0x8007},
    {"SHL", VX, 0x800E},
    {"SNE", VX_VY, 0x9000},
    {"LD", I_ADDR, 0xA000},
    {"JP", V0_ADDR, 0xB000},
    {"RND", VX_BYTE, 0xC000},
    {"DRW", VX_VY_N, 0xD000},
    {"SKP", VX, 0xE09E},
    {"SKNP", VX, 0xE0A1},
    {"LD", VX_DT, 0xF007},
    {"LD", VX_K, 0xF00A},
    {"LD", DT_VX, 0xF015},
    {"LD", ST_VX, 0xF018},
    {"ADD", I_VX, 0xF01E},
    {"LD", F_VX, 0xF029},
    {"LD", B_VX, 0xF033},
    {"LD", I_VX, 0xF055},
    {"LD", VX_I, 0xF065},
};

static const AsmOp *find_op(const char *mnemonic, Shape shape) {
    for (size_t i = 0; i < sizeof(ops) / sizeof(AsmOp); i++) {
        if (ops[i].shape == shape && strcmp(ops[i].mnemonic, mnemonic) == 0)
            return &ops[i];
    }
    return nullptr;
}

static uint16_t encode(const AsmOp *op, int x, int y, int n, int nn, int nnn) {
    switch (op->shape) {
        case NONE:
            return op->base;
            break;
        case ADDR:
        case V0_ADDR:
        case I_ADDR:
            return op->base | (nnn & 0x0FFF);
            break;
        case VX_BYTE:
            return op->base | ((x & 0xF) << 8) | (nn & 0xFF);
            break;
        case VX_VY:
            return op->base | ((x & 0xF) << 8) | ((y & 0xF) << 4);
            break;
        case VX_VY_N:
            return op->base | ((x & 0xF) << 8) | ((y & 0xF) << 4) | (n & 0xF);
            break;
        default:
            return op->base | ((x & 0xF) << 8);
    }
}

static bool is_reg(const char *s) {
    int d = -1;
    if (s[1] >= '0' && s[1] <= '9') d = s[1] - '0';
    if (s[1] >= 'a' && s[1] <= 'f') d = s[1] - 'a' + 10;
    if (s[1] >= 'A' && s[1] <= 'F') d = s[1] - 'A' + 10;
    return s[0] == 'V' && d >= 0 && s[2] == '\0';
}

static int reg(const char *s) {
    if (s[1] >= '0' && s[1] <= '9') return s[1] - '0';
    if (s[1] >= 'a' && s[1] <= 'f') return s[1] - 'a' + 10;
    return s[1] - 'A' + 10;
}

static Shape get_shape(char tokens[][16], int n) {
    const char *a = n > 1 ? tokens[1] : "";
    const char *b = n > 2 ? tokens[2] : "";

    if (n == 1) return NONE;
    if (n == 2 && is_reg(a)) return VX;
    if (n == 2) return ADDR;
    if (n == 4 && is_reg(a) && is_reg(b)) return VX_VY_N;
    if (strcmp(tokens[0], "JP") == 0 && strcmp(a, "V0") == 0) return V0_ADDR;
    if (is_reg(a) && is_reg(b)) return VX_VY;
    if (is_reg(a) && strcmp(b, "DT") == 0) return VX_DT;
    if (is_reg(a) && strcmp(b, "K") == 0) return VX_K;
    if (is_reg(a) && strcmp(b, "[I]") == 0) return VX_I;
    if (strcmp(a, "DT") == 0 && is_reg(b)) return DT_VX;
    if (strcmp(a, "ST") == 0 && is_reg(b)) return ST_VX;
    if (strcmp(a, "F") == 0 && is_reg(b)) return F_VX;
    if (strcmp(a, "B") == 0 && is_reg(b)) return B_VX;
    if ((strcmp(a, "I") == 0 || strcmp(a, "[I]") == 0) && is_reg(b)) return I_VX;
    if (strcmp(a, "I") == 0) return I_ADDR;
    if (is_reg(a)) return VX_BYTE;
    return NONE;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: assembler <file>\n");
        return -1;
    }

    char buff[256];

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Could not open file %s\n", argv[1]);
        return -1;
    }

    char out_path[256];
    const char *dot = strrchr(argv[1], '.');
    size_t stem = dot ? (size_t)(dot - argv[1]) : strlen(argv[1]);
    snprintf(out_path, sizeof out_path, "%.*s.ch8", (int)stem, argv[1]);

    FILE *out = fopen(out_path, "wb");
    if (!out) {
        fprintf(stderr, "Could not create output file %s\n", out_path);
        fclose(fp);
        return -1;
    }

    while (fgets(buff, sizeof(buff), fp) != nullptr) {
        char *p = buff;
        char tokens[8][16];
        int n = 0;
        while (*p != '\0' && *p != '\n' && *p != ';' && n < 8) {
            while (*p == ' ' || *p == '\t' || *p == ',')
                p++;
            if (*p == '\0' || *p == '\n' || *p == ';')
                break;
            int len = 0;
            while (*p != '\0' && *p != '\n' && *p != ';' &&
                *p != ' ' && *p != '\t' && *p != ',') {
                if (len < 15)
                    tokens[n][len++] = *p;
                p++;
            }
            tokens[n][len] = '\0';
            n++;
        }
        if (n == 0)
            continue;

        Shape shape = get_shape(tokens, n);
        const AsmOp *op = find_op(tokens[0], shape);
        if (!op) {
            fprintf(stderr, "Bad instruction: %s", buff);
            fclose(out);
            fclose(fp);
            return -1;
        }

        int x = 0, y = 0, nibble = 0, nn = 0, nnn = 0;
        switch (shape) {
            case VX:
            case VX_DT:
            case VX_K:
            case VX_I:
                x = reg(tokens[1]);
                break;
            case DT_VX:
            case ST_VX:
            case F_VX:
            case B_VX:
            case I_VX:
                x = reg(tokens[2]);
                break;
            case VX_BYTE:
                x = reg(tokens[1]);
                nn = (int) strtol(tokens[2], nullptr, 0);
                break;
            case VX_VY:
                x = reg(tokens[1]);
                y = reg(tokens[2]);
                break;
            case VX_VY_N:
                x = reg(tokens[1]);
                y = reg(tokens[2]);
                nibble = (int) strtol(tokens[3], nullptr, 0);
                break;
            case ADDR:
                nnn = (int) strtol(tokens[1], nullptr, 0);
                break;
            case V0_ADDR:
            case I_ADDR:
                nnn = (int) strtol(tokens[2], nullptr, 0);
                break;
            default:
                break;
        }

        const uint16_t word = encode(op, x, y, nibble, nn, nnn);

        const unsigned char bytes[2] = {word >> 8, word & 0xFF};
        fwrite(bytes, 1, 2, out);
    }

    printf("Written to %s\n", out_path);
    fclose(fp);
    fclose(out);
    return 0;
}
