#include "defuser.h"
#include <string.h>
#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t low  = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);

    uint32_t a = (uint32_t)((high - low) * 3u + 0xc922u);

    uint32_t b = (uint32_t)(low + 8ull * (challenge >> 16)) ^ 0xde98u;

    values[0] = (uint32_t)((a ^ 0x64aeu) + 2u * b);

    count = 1;
    return write_answer(out, cap, values, count);
}
int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    values[0] = (uint32_t)((challenge >> 46) & 0xfu) ^ 0x7u;

    for (uint32_t i = 1; i < 6; ++i) {
        uint32_t t    = (uint32_t)((challenge >> (i + 12u)) & 0x3fu) + 3u;
        uint32_t prev = 5u * values[i - 1] - i;
        values[i] = t ^ prev;
    }

    count = 6;
    return write_answer(out, cap, values, count);
}
int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};

    uint32_t sel = (uint32_t)challenge & 0x7u;
    uint32_t x   = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 3);

    uint32_t y;
    switch (sel) {
    case 0: y = (2u * x) ^ 0x7bcfu;        break;
    case 1: y = 5u * x + 0x3ca5u;          break;
    case 2: y = (x ^ 0x7cf1u) + (x >> 2);  break;
    case 3: y = (x ^ 0xf74fu) + (x >> 2);  break;
    case 4: y = 3u * x + 0xc406u;          break;
    case 5: y = 3u * x + 0xef96u;          break;
    case 6: y = (2u * x) ^ 0xf600u;        break;
    case 7: y = 5u * x + 0x0fu;            break;
    default: return 1;
    }

    values[0] = sel;
    values[1] = y;
    return write_answer(out, cap, values, 2);
}


static uint32_t p4_walk(uint32_t target, uint32_t lo, uint32_t hi, uint32_t key) {
    uint32_t mid = lo + (hi - lo) / 2u;

    if (mid == target)
        return (mid + 0x9eu) ^ key;

    uint32_t mix = mid + key;
    if (target < mid) {
        uint32_t r = p4_walk(target, lo, mid - 1u, key);
        return (3u * r) ^ mix ^ 0x1cu;
    }
    uint32_t r = p4_walk(target, mid + 1u, hi, key);
    return (3u * r) ^ mix ^ 0x1eu;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};

    uint32_t target = (uint32_t)challenge & 0x7fu;
    uint32_t key    = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 7);

    values[0] = p4_walk(target, 0u, 0x7fu, key);

    size_t count = 1;
    return write_answer(out, cap, values, count);
}

struct p5_node {
    int      next;
    uint32_t d;
    uint32_t c;
};

static const struct p5_node p5_nodes[12] = {
    /*  0  0x405ba0 */ {  9, 0x0314u, 0xaf31u },
    /*  1  0x405bb0 */ {  5, 0x0457u, 0x5f19u },
    /*  2  0x405bc0 */ { -1, 0x004fu, 0xdfb9u },
    /*  3  0x405bd0 */ { 10, 0x024eu, 0x2908u },
    /*  4  0x405be0 */ {  8, 0x0935u, 0x5e46u },
    /*  5  0x405bf0 */ {  3, 0x0522u, 0x2715u },
    /*  6  0x405c00 */ {  4, 0x0868u, 0x72d1u },
    /*  7  0x405c10 */ {  6, 0x063fu, 0xe1e8u },
    /*  8  0x405c20 */ { 11, 0x0787u, 0xe268u },
    /*  9  0x405c30 */ {  1, 0x010eu, 0xe69au },
    /* 10  0x405c40 */ {  2, 0x0a46u, 0x7f59u },
    /* 11  0x405c50 */ {  0, 0x0b49u, 0xb5aeu },
};

static const int p5_heads[8] = { 7, 6, 4, 8, 11, 0, 9, 1 };

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};

    uint32_t start = (uint32_t)challenge & 0x7u;
    uint32_t steps = ((uint32_t)(challenge >> 3) & 0x3u) + 1u;
    uint32_t acc   = ((uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32)) ^ 0xbc12u;

    int      idx    = p5_heads[start];
    uint32_t last_d = 0;

    for (uint32_t k = 0; k < steps && idx >= 0; ++k) {
        acc    = acc * 3u + p5_nodes[idx].c;
        acc   ^= p5_nodes[idx].d;
        last_d = p5_nodes[idx].d;
        idx    = p5_nodes[idx].next;
    }

    values[0] = acc;
    values[1] = last_d;

    size_t count = 2;
    return write_answer(out, cap, values, count);
}


static inline uint32_t rol32(uint32_t v, unsigned n) { return (v << n) | (v >> (32u - n)); }
static inline uint32_t ror32(uint32_t v, unsigned n) { return (v >> n) | (v << (32u - n)); }


static uint32_t p6_op(uint32_t idx, uint32_t x) {
    switch (idx) {
    case 0:  return (x + 0xaf07u) ^ (x >> 2);         /* p6_op_3 */
    case 1:  return rol32(x, 2) ^ 0x92abu;            /* p6_op_4 */
    case 2:  return (x ^ 0x186du) + (x << 5);         /* p6_op_2 */
    case 3:  return ((x >> 1) ^ x) + 0x8eu;           /* p6_op_0 */
    case 4:  return ((x >> 1) ^ x) + 0x8eu;           /* p6_op_0（重复表项） */
    case 5:  return (x ^ 0x186du) + (x << 5);         /* p6_op_2（重复表项） */
    case 6:  return ((x << 7) + 0xd5ebu) ^ (7u * x);  /* p6_op_1 */
    case 7:  return ror32(x, 1);                      /* p6_op_5 */
    default: return x;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};

    uint32_t x = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    uint32_t i = (uint32_t)challenge & 0x7u;
    uint32_t j = ((uint32_t)challenge >> 3) & 0x7u;

    uint32_t t = p6_op(i, x) ^ 0x92c0u;
    uint32_t r = p6_op(j, t);

    values[0] = r + 0xcee8u;

    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t A = (uint32_t)(challenge >> 32) ^ (uint32_t)challenge;
    uint32_t B = (uint32_t)(context >> 32) ^ (uint32_t)context;
    uint32_t c = (uint32_t)context & 0x3u;

    uint32_t mix = (rol32(A ^ B, 1) + 0xed19u) ^ 0xe91cu;

    uint32_t v0 = c;
    uint32_t v1 = mix ^ (c * 0xed19u) ^ (uint32_t)(context >> 16);
    uint32_t v2 = v1 ^ (B + c + 0xe91cu + (A >> 1));

    char head[37];
    memset(head, 'A', 32);
    memcpy(head + 32, "OPEN", 4);
    head[36] = '\0';

    int n = snprintf(out, cap, "%s %" PRIu32 " %" PRIu32 " %" PRIu32,
                     head, v0, v1, v2);
    return (n < 0 || (size_t)n >= cap) ? 1 : 0;
}
