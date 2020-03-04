#ifndef _OPENAPIXX_ASCII_H_
#define _OPENAPIXX_ASCII_H_

#include <cstdint>

#define OAASCII_CTL     0b10000000
#define OAASCII_CAP     0b01000000
#define OAASCII_LOW     0b00100000
#define OAASCII_NUM     0b00010000
#define OAASCII_SYMB    0b00001000
#define OAASCII_BLK     0b00000100

#define OAASCII_ALPH    (OAASCII_CAP | OAASCII_LOW)
#define OAASCII_PRT     (OAASCII_CAP | OAASCII_LOW | OAASCII_NUM | OAASCII_SYMB)
#define OAASCII_ALPH_NUM    (OAASCII_CAP | OAASCII_LOW | OAASCII_NUM)

namespace OpenAPIxx {

class OA_ASCII {
public:
    // ��С����ĸ
    static bool inline isAlphbate(char ch) { return m_charMap[ch] & OAASCII_ALPH; }

    // ����0~9
    static bool inline isNumber(char ch) { return m_charMap[ch] & OAASCII_NUM; }

    // ��̖
    static bool inline isSymbol(char ch) { return m_charMap[ch] & OAASCII_SYMB; }

    // �����ַ���ASCII��0~31, 127
    static bool inline isControl(char ch) { return m_charMap[ch] & OAASCII_CTL; }

    // ����ĸ
    static bool inline isUpper(char ch) { return m_charMap[ch] & OAASCII_CAP; }

    // С����ĸ
    static bool inline isLower(char ch) { return m_charMap[ch] & OAASCII_LOW; }

    // ��С����ĸ�͔���
    static bool inline isAlphbateOrNumber(char ch) { return m_charMap[ch] & OAASCII_ALPH_NUM; }

    // �ɴ�ӡ�ַ��������������ַ���������\r��\n��\t
    static bool inline isPrintable(char ch) { return m_charMap[ch] & OAASCII_PRT; }

    // �ո�\r��\n��\t
    static bool inline isBlank(char ch) { return m_charMap[ch] & OAASCII_BLK; }

private:
    static uint8_t m_charMap[128];
};

} // namepsace

typedef OpenAPIxx::OA_ASCII OAASCII;

#endif//_OPENAPIXX_ASCII_H_
