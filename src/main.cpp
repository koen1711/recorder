
#include <string>
#include "dashboard/dashboard.h"

struct a {
    int64_t a;
};

int main(int argc, char** argv)
{
    auto* dashboard = new Dashboard();
    dashboard->startServer();




    return 0;
}
