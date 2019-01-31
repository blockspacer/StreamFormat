#include <sf/sformat.hpp>

using namespace sf;
using namespace std;

int main(int argc, char** argv)
{
    if (argc > 2)
    {
        string result;
        auto [pos, a, b] = sscan<int, int>(argv[1], "{}{1}");
        if (pos != decltype(pos)(-1))
        {
            sscan(argv[1] + pos, result);
        }
        else
        {
            result = sprint("{0} + {} = {2}", a, b, a + b);
        }
        if (result == argv[2])
        {
            print("Success.\n");
        }
    }
    return 0;
}
