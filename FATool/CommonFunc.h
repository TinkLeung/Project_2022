#ifndef COMMONFUNC_H
#define COMMONFUNC_H
#include <QString>
#include <QList>

#define BIT0   0x1
#define BIT1   0x2
#define BIT2   0x4
#define BIT3   0x8
#define BIT4   0x10
#define BIT5   0x20
#define BIT6   0x40
#define BIT7   0x80

#define ERROR_SUCCESS_STATUS        0x0
#define MAX_DRIVE_NUM               64
#define MAX_TEXT_LENGTH             100

//Every info length
#define FLASH_ID_DATA_LENGTH        0X200
#define SMART_DATA_LENGTH           0x200
#define EVENT_LOG_DATA_LENGTH       0x1000
#define ERROR_ENTRY_LENGTH          0x40
#define IDY_NAMESPACE_INFO_LENGTH   0X1000
#define IDY_CONTROLLER_INFO_LENGTH  0X1000


typedef struct{
    QString Description;
    QString RawValue;
}SPEC_ITEM,*PSPEN_ITEM;

union uint128b
{
    unsigned char bytes[16];
    unsigned int  words[8];
};

typedef union uint128b uint128b_t;

uint128b_t le128_to_cpu(unsigned char *data);
char *uint128b_to_string(uint128b_t val);
char *flipAndCodeBytes(const char * str, int pos, int flip, char * buf);

long double int128_to_double(unsigned char *data);
int char_to_int(char data);
//QString FillSpace(QString str, int maxlen);

QList<SPEC_ITEM>* ParseSmartData(unsigned char *data);
QList<SPEC_ITEM>* ParseErrorLog(unsigned char *data,int entries);
QList<SPEC_ITEM>* ParseIdyCtrlData(unsigned char *data);
QList<SPEC_ITEM>* ParseIdyCtrNSlData(unsigned char *data);

#endif // COMMONFUNC_H
