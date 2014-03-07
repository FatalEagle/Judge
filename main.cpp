#include "resource.h"

int main(int argc, char *argv[])
{
    DEFAULT_RNG.seed(time(nullptr));
    std::ios_base::sync_with_stdio(false);
    if(argc<2)
    {
        std::cout<<"Error: Not enough command line arguments"<<std::endl;
        return -1;
    }
    std::ofstream fout;
    if(argc>=5)
        fout.open(argv[5], std::ostream::app);
    std::ostream& outstream=*((argc>=5)?static_cast<std::ostream*>(&fout):
                             static_cast<std::ostream*>(&std::cout));
    Judge judge(outstream);
    int finalscore=judge.run(argv[1], argv[2], (argc>=3)?(argv[3]):(std::string()), (argc>=4)?(argv[4]):(std::string()));
    outstream<<finalscore<<" points awarded."<<std::endl;
    return finalscore;
}
