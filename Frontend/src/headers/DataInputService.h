#ifndef DATAINPUTSERVICE_H
#define DATAINPUTSERVICE_H

#include <QString>
#include <QList>

namespace DataInputService{

bool emailValid(QString login);

bool passwordValidLength(QString password);

bool passwordValid(QString password);

bool passwordValidCharacters(QString password);

bool tagValid(QString tag);

bool tagValidCharacters(QString tag);

bool nameValid(QString name);

} //namespace

bool DataInputService::nameValid(QString name){
    return name.length() >= 4 && name.length() <= 20;
}

bool DataInputService::emailValid(QString login){
    QString domain = "@gmail.com";
    QString beforeDomain;
    for(auto el: login){
        if(el == '@'){
            return beforeDomain + domain == login && !beforeDomain.isEmpty();
        }
        else if (!el.isLetterOrNumber()) return false;
        beforeDomain += el;
    }
    return false;
}

bool DataInputService::passwordValidLength(QString password){
    int minPasswordLength = 8;
    int maxPasswordLenth = 22;

    return password.length() >= minPasswordLength && password.length() <= maxPasswordLenth;
}

bool DataInputService::passwordValid(QString password){
    return DataInputService::passwordValidLength(password) && DataInputService::passwordValidCharacters(password);
}

bool DataInputService::passwordValidCharacters(QString password){
    QList<QChar> symbols;
    symbols << '!' << '$' << '_' << '+';
    for(auto el: password){
        if (!el.isLetterOrNumber() && !symbols.contains(el)) return false;
    }
    return true;
}

bool DataInputService::tagValid(QString tag){
    if(!DataInputService::tagValidCharacters(tag)) return false;

    int minTagLength = 4;
    int maxTagLenth = 11;

    return tag.length() >= minTagLength && tag.length() <= maxTagLenth;
}

bool DataInputService::tagValidCharacters(QString tag){
    if(tag.isEmpty()) return false;

    if(tag.front().isLetterOrNumber() == false) return false;
    QChar prev = 'a';

    for(auto el: tag){
        if(el.isPunct()){
            if(el != '_' || prev == '_') return false; // two _ in a row
        }else {
            if(!el.isLetterOrNumber()) return false;
        }
        prev = el;
    }

    return true;
}

#endif // DATAINPUTSERVICE_H
