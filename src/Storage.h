#include <LittleFS.h>
class Storage
{
    private:
    public:
        boolean init(void);
        void write(String file_name, char text[]);
        String get(const char *fileName);
};
