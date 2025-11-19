#ifndef IAPP_H
#define IAPP_H

class IApp {
  public:
    virtual IApp& port(int) = 0;
    virtual IApp& multithreaded() = 0;
    virtual void run() = 0;
    virtual ~IApp() = default;
};

#endif // IAPP_H
