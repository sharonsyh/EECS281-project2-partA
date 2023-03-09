// Project Identifier: AD48FB4835AF347EB0CA8009E24C3B13F8519882

#include <vector>
#include <iostream>
#include <algorithm> 
#include <getopt.h>
#include <string>
#include <queue>

using namespace std;

using namespace std;

class Stars{

    public:

        enum State {
                Initial,
                Seen_one,
                Seen_two,
                Maybe_better 
        };

        struct deployment{

            int timestamp;
            // string sith_or_jedi;
            uint32_t general_id;
            int force_sens;
            int num_troops; 
            uint32_t order;

        } deployments;

        struct CompareJedi{
            bool operator()(deployment const& j1, deployment const& j2)
            {
                if(j1.force_sens == j2.force_sens) return (j1.order > j2.order);
                else return (j1.force_sens  > j2.force_sens);
            }
        };

        struct CompareSith{
            bool operator()(deployment const& s1, deployment const& s2)
            {
                if(s1.force_sens == s2.force_sens) return (s1.order > s2.order);
                else return (s1.force_sens < s2.force_sens);
            }
        };

        struct general{
            int jedi_total = 0;
            int sith_total = 0;
            int lost_troops = 0;
        };

        struct planet{

            State attack_mode;
            State ambush_mode;
            deployment best_ambush_sith;
            deployment best_attack_sith;
            deployment best_ambush_jedi;
            deployment best_attack_jedi;
            deployment maybe_best_sith;
            deployment maybe_best_jedi;


            //Median troops lost on planet <PLANET_ID> at time <TIMESTAMP> is <MED_TROOPS_LOST>.
            //fast median finder
            priority_queue<int> s;
            priority_queue<int,vector<int>,greater<int> > g;
            int median = -1;
            

            //pq for jedi -- min priority queue
            priority_queue<deployment, vector<deployment>,  CompareJedi> jedi; 
            //pq for sith -- max priority queue
            priority_queue<deployment, vector<deployment>, CompareSith> sith;

        };


        void get_options(int argc, char** argv);
        void read_file();

    private:

        


};
