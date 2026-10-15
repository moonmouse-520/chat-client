#include "global.h"

QString gate_url_prefix = "";

std::function<void(QWidget*)> repolish = [](QWidget* w){
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString = [](QString input) {
    auto result = input;
    int length = result.length();
    length = length % 255;
    for(int i = 0; i < length;++i){
        //对每个字符进行异或操作
        //这里每一个字符都是ASCII, 因此要转换为QChar
        result[i] = QChar(static_cast<ushort>(input[i].unicode() ^ static_cast<ushort>(length)));
    }
    return result;
};