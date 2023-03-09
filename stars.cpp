// Project Identifier: AD48FB4835AF347EB0CA8009E24C3B13F8519882

#include <iostream>
using std::cin;
using std::cout;
using std::endl;
#include <vector>
#include <stack>
using std::vector;
#include "stars.h"
#include "P2random.h"

struct Options{
        bool verbose = false;
        bool median = false;
        bool general_eval = false;
        bool watcher = false;
} Opt;

void Stars::get_options(int argc, char** argv){

    int c;

    while (1) {

        int option_index = 0;
        static struct option long_options[] = {
            {"verbose",     no_argument, 0, 'v'},
            {"median",  no_argument, 0, 'm'},
            {"general-eval",  no_argument, 0, 'g'},
            {"watcher", no_argument, 0, 'w'},
        };

        c = getopt_long(argc, argv, "vmgw",
                long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'v':
                Opt.verbose = true;
                break;
            case 'm':
                Opt.median = true;
                break;
            case 'g':
                Opt.general_eval = true;
                break;
            case 'w':
                Opt.watcher = true;
                break;

            default:
                exit(1);
        }
    }
}

void Stars::read_file(){

    string new_line;
    getline(cin, new_line);

    string inputMode;

    cin >> new_line >> inputMode ;

    uint32_t num_generals;

    cin >> new_line >> num_generals;

    vector<general> generals;
    generals.resize(num_generals);

    uint32_t num_planets;

    cin >> new_line >> num_planets;

    vector<deployment> deployments;
    stringstream ss;
    vector<planet> planets;
    int battle_count = 0;
    planets.resize(num_planets);

    if(inputMode == "PR") {

        uint32_t r_seed;
        uint32_t num_deployments;
        uint32_t arrival_rate;

        cin >> new_line >> r_seed;
        cin >> new_line >> num_deployments;
        cin >> new_line >> arrival_rate;

        P2random::PR_init(ss, r_seed, num_generals, num_planets, num_deployments, arrival_rate);

    }

    // int index;
    
    istream &inputStream = inputMode == "PR" ? ss : cin;
    cout << "Deploying troops...\n";

    deployment dply;
    char letter;
    // stack <int> times;
    int previous = 0;
    uint32_t  i = 0;
    uint32_t planet_num;
    // uint32_t timestamp;
    string sith_or_jedi;
    // bool battle_on = false;

    for(size_t i = 0 ; i < num_planets ; i ++) {
        planets[i].attack_mode = State::Initial;
        planets[i].ambush_mode = State::Initial;
    }

    while (inputStream >> dply.timestamp) {
    // Process this deployment, use PQs, initiate battles (if possible), etc.
        inputStream >> sith_or_jedi >> letter >> dply.general_id >> letter >> planet_num >> letter >> dply.force_sens >> letter >> dply.num_troops;
        dply.order = i;
        i++;

        if(dply.general_id >= num_generals || static_cast<int>(dply.general_id) < 0) {
            cerr << "Invalid general ID" << endl;
            exit(1);
        }
        if(planet_num >= num_planets || static_cast<int>(planet_num) < 0 ) {
            cerr << "Invalid planet ID" << endl;
            exit(1);
        }
        if(dply.force_sens <= 0) {
            cerr << "Invalid force sensitivity level" << endl;
            exit(1);
        }
        if(dply.num_troops <= 0) {
            cerr << "Invalid number of troops" << endl;
            exit(1);
        }
        if(previous > dply.timestamp){
            cerr << "Invalid decreasing timestamp" << endl;
            exit(1);
        }
        // cout << 
        if(Opt.median && previous != dply.timestamp) {
            for(size_t j = 0 ; j < num_planets ; j ++) {
                if(planets[j].median != -1 && (planets[j].s.size() != 0 || planets[j].g.size() != 0)) {
                    cout << "Median troops lost on planet " << j << " at time " << previous << " is " << planets[j].median << ".\n";
                }
            }
        }

        // battle_on = false;

        previous = dply.timestamp;

        if(sith_or_jedi == "JEDI") {

            planets[planet_num].jedi.push(dply);
            generals[dply.general_id].jedi_total += dply.num_troops;
            
            if( Opt.watcher) {
                
                //attack
                if(planets[planet_num].jedi.size() == 1 && planets[planet_num].attack_mode == State::Initial) {

                    planets[planet_num].attack_mode = State::Seen_one; // attack mode : jedi first, sith comes

                    planets[planet_num].best_attack_jedi = dply;

                }
                else if(planets[planet_num].attack_mode == State::Seen_one && planets[planet_num].best_attack_jedi.force_sens > dply.force_sens) {
                    planets[planet_num].best_attack_jedi = dply;
                }
                else if(planets[planet_num].attack_mode == State::Seen_two) {
                    planets[planet_num].maybe_best_jedi = dply;
                    planets[planet_num].attack_mode = State::Maybe_better;
                }
                else if(planets[planet_num].attack_mode  == State::Maybe_better && planets[planet_num].maybe_best_jedi.force_sens > dply.force_sens) { //sith jedi sith now jedi
                    planets[planet_num].maybe_best_jedi = dply;
                }

                // ambush {
                if(planets[planet_num].ambush_mode == State::Seen_one && planets[planet_num].best_ambush_sith.force_sens >= dply.force_sens) {
                    planets[planet_num].best_ambush_jedi = dply;
                    planets[planet_num].ambush_mode = State::Seen_two;
                }

                if((planets[planet_num].ambush_mode == State::Seen_two || planets[planet_num].ambush_mode == State::Maybe_better) 
                && planets[planet_num].best_ambush_jedi.force_sens > dply.force_sens) { //sith jedi jedi
                    planets[planet_num].best_ambush_jedi = dply;
                }

                if(planets[planet_num].ambush_mode  == State::Maybe_better) { //sith jedi sith now jedi

                    if(planets[planet_num].maybe_best_sith.force_sens - dply.force_sens
                        > planets[planet_num].best_ambush_sith.force_sens - planets[planet_num].best_ambush_jedi.force_sens) {

                        planets[planet_num].best_ambush_jedi = dply;
                        planets[planet_num].best_ambush_sith = planets[planet_num].maybe_best_sith;
                        planets[planet_num].ambush_mode = State::Seen_two;
                    }
                }
            }
        }
        else {

            planets[planet_num].sith.push(dply);
            generals[dply.general_id].sith_total += dply.num_troops;
            
            //initialize
            if(Opt.watcher) {

                if(planets[planet_num].sith.size() == 1 && planets[planet_num].ambush_mode == State::Initial) {

                    planets[planet_num].ambush_mode = Seen_one;
                    planets[planet_num].best_ambush_sith = dply;
                    
                }
                if(planets[planet_num].ambush_mode == State::Seen_one && planets[planet_num].best_ambush_sith.force_sens < dply.force_sens) {
                    planets[planet_num].best_ambush_sith = dply;
                }

                if(planets[planet_num].ambush_mode == State::Seen_two && planets[planet_num].best_ambush_sith.force_sens < dply.force_sens) {
                    planets[planet_num].maybe_best_sith = dply;
                    planets[planet_num].ambush_mode = State::Maybe_better;
                }
                if(planets[planet_num].ambush_mode  == State::Maybe_better && planets[planet_num].maybe_best_sith.force_sens < dply.force_sens) { //sith jedi sith now jedi
                    planets[planet_num].maybe_best_sith = dply;
                }


                //attack
                if(planets[planet_num].attack_mode == State::Seen_one && planets[planet_num].best_attack_jedi.force_sens <= dply.force_sens) {
                    // planets[planet_num].best_attack_sith = dply;
                    planets[planet_num].attack_mode = State::Seen_two;
                    planets[planet_num].best_attack_sith = dply;
                }
                else if(planets[planet_num].attack_mode == State::Seen_two || planets[planet_num].attack_mode == State::Maybe_better)  {
                    if( planets[planet_num].best_attack_sith.force_sens < dply.force_sens) {
                        planets[planet_num].best_attack_sith = dply;
                    }

                    if(planets[planet_num].attack_mode  == State::Maybe_better) { //jedi sith jedi now sith
                        if(dply.force_sens - planets[planet_num].maybe_best_jedi.force_sens
                          > planets[planet_num].best_attack_sith.force_sens - planets[planet_num].best_attack_jedi.force_sens) {
                            planets[planet_num].best_attack_sith = dply;
                            planets[planet_num].best_attack_jedi = planets[planet_num].maybe_best_jedi;
                        }
                    }
                }
                
            }
        }


        //when either one among jedi and sith is gone, stop
        //when jedi's force sensitivity > sith's force sensitivity, stop

    while(planets[planet_num].jedi.size() > 0 && planets[planet_num].sith.size() > 0 &&
            planets[planet_num].jedi.top().force_sens <= planets[planet_num].sith.top().force_sens) {
            // battle_on = true;
            battle_count++;
            
            if(planets[planet_num].jedi.top().num_troops > planets[planet_num].sith.top().num_troops) {

                if(Opt.verbose) {

                    cout << "General " << planets[planet_num].sith.top().general_id 
                    << "'s battalion attacked General " << planets[planet_num].jedi.top().general_id 
                    << "'s battalion on planet " << planet_num << ". "<< planets[planet_num].sith.top().num_troops * 2 << " troops were lost.\n";
                }

                generals[planets[planet_num].sith.top().general_id].lost_troops += planets[planet_num].sith.top().num_troops;
                generals[planets[planet_num].jedi.top().general_id].lost_troops += planets[planet_num].sith.top().num_troops;

                if(Opt.median) {

                    //when timestamp changes, print the median when median mode

                    if(planets[planet_num].s.size() == 0 && planets[planet_num].g.size() == 0) {
                        // cout << "[DEBUG] planets\n"<< planet_num << "].median = " << planets[planet_num].median << endl;
                        planets[planet_num].s.push(planets[planet_num].sith.top().num_troops * 2);
                        planets[planet_num].median = planets[planet_num].sith.top().num_troops * 2;
                    }

                    else if (planets[planet_num].s.size() > planets[planet_num].g.size()) {

                        if (planets[planet_num].sith.top().num_troops * 2 < planets[planet_num].s.top()) {   
                            planets[planet_num].g.push(planets[planet_num].s.top());
                            planets[planet_num].s.pop();
                            planets[planet_num].s.push(planets[planet_num].sith.top().num_troops * 2);
                        }
                        else planets[planet_num].g.push(planets[planet_num].sith.top().num_troops * 2);
            
                        planets[planet_num].median = (planets[planet_num].s.top() + planets[planet_num].g.top())/2;
                    }
                    else if (planets[planet_num].s.size() == planets[planet_num].g.size()) {

                        if (planets[planet_num].sith.top().num_troops * 2 < planets[planet_num].s.top()) {
                            planets[planet_num].s.push(planets[planet_num].sith.top().num_troops * 2);
                            planets[planet_num].median = planets[planet_num].s.top();
                        }
                        else {
                            planets[planet_num].g.push(planets[planet_num].sith.top().num_troops * 2);
                            planets[planet_num].median = planets[planet_num].g.top();
                        }
                    }
                    else {
                        if (planets[planet_num].sith.top().num_troops * 2 > planets[planet_num].g.top()) {
                            planets[planet_num].s.push(planets[planet_num].g.top());
                            planets[planet_num].g.pop();
                            planets[planet_num].g.push(planets[planet_num].sith.top().num_troops * 2);
                        }
                        else planets[planet_num].s.push(planets[planet_num].sith.top().num_troops * 2);
            
                        planets[planet_num].median = (planets[planet_num].s.top() + planets[planet_num].g.top())/2;
                    }

                }
                deployment temp;
                temp.num_troops = planets[planet_num].jedi.top().num_troops - planets[planet_num].sith.top().num_troops;
                temp.force_sens = planets[planet_num].jedi.top().force_sens;
                temp.general_id = planets[planet_num].jedi.top().general_id;
                temp.order = planets[planet_num].jedi.top().order;
                // temp.sith_or_jedi = planets[planet_num].jedi.top().sith_or_jedi;
                temp.timestamp = planets[planet_num].jedi.top().timestamp;

                planets[planet_num].jedi.pop();
                planets[planet_num].jedi.push(temp);

                planets[planet_num].sith.pop();

            }

            else if(planets[planet_num].sith.top().num_troops > planets[planet_num].jedi.top().num_troops) {


                if(Opt.verbose) {

                    cout << "General " << planets[planet_num].sith.top().general_id 
                    << "'s battalion attacked General " << planets[planet_num].jedi.top().general_id 
                    << "'s battalion on planet " << planet_num << ". "<< planets[planet_num].jedi.top().num_troops * 2 << " troops were lost.\n";
                }

                generals[planets[planet_num].sith.top().general_id].lost_troops += planets[planet_num].jedi.top().num_troops;
                generals[planets[planet_num].jedi.top().general_id].lost_troops += planets[planet_num].jedi.top().num_troops;

                if(Opt.median) {

                    //when timestamp changes, print the median when median mode

                    if(planets[planet_num].s.size() == 0 && planets[planet_num].g.size() == 0) {
                        planets[planet_num].s.push(planets[planet_num].jedi.top().num_troops * 2);
                        planets[planet_num].median = planets[planet_num].jedi.top().num_troops * 2;
                    }

                    else if (planets[planet_num].s.size() > planets[planet_num].g.size()) {

                        if (planets[planet_num].jedi.top().num_troops * 2 < planets[planet_num].s.top()) {   
                            planets[planet_num].g.push(planets[planet_num].s.top());
                            planets[planet_num].s.pop();
                            planets[planet_num].s.push(planets[planet_num].jedi.top().num_troops * 2);
                        }
                        else
                            planets[planet_num].g.push(planets[planet_num].jedi.top().num_troops * 2);
            
                        planets[planet_num].median = (planets[planet_num].s.top() + planets[planet_num].g.top())/2;
                    }
                    else if (planets[planet_num].s.size() == planets[planet_num].g.size())
                    {
                        if (planets[planet_num].jedi.top().num_troops * 2 < planets[planet_num].s.top()) {
                            planets[planet_num].s.push(planets[planet_num].jedi.top().num_troops * 2);
                            planets[planet_num].median = planets[planet_num].s.top();
                        }
                        else {
                            planets[planet_num].g.push(planets[planet_num].jedi.top().num_troops * 2);
                            planets[planet_num].median = planets[planet_num].g.top();
                        }
                    }
                    else {
                        if (planets[planet_num].jedi.top().num_troops * 2 > planets[planet_num].g.top()) {
                            planets[planet_num].s.push(planets[planet_num].g.top());
                            planets[planet_num].g.pop();
                            planets[planet_num].g.push(planets[planet_num].jedi.top().num_troops * 2);
                        }
                        else planets[planet_num].s.push(planets[planet_num].jedi.top().num_troops * 2);
            
                        planets[planet_num].median = (planets[planet_num].s.top() + planets[planet_num].g.top())/2;
                    }

                }

                deployment temp;
                temp.num_troops = planets[planet_num].sith.top().num_troops - planets[planet_num].jedi.top().num_troops;
                temp.force_sens = planets[planet_num].sith.top().force_sens;
                temp.general_id = planets[planet_num].sith.top().general_id;
                temp.order = planets[planet_num].sith.top().order;
                // temp.sith_or_jedi = planets[planet_num].sith.top().sith_or_jedi;
                temp.timestamp = planets[planet_num].sith.top().timestamp;

                planets[planet_num].sith.pop();
                planets[planet_num].sith.push(temp);
                
                planets[planet_num].jedi.pop();
            }

            else {

                if(Opt.verbose) {

                    cout << "General " << planets[planet_num].sith.top().general_id 
                    << "'s battalion attacked General " << planets[planet_num].jedi.top().general_id 
                    << "'s battalion on planet " << planet_num << ". "<< planets[planet_num].jedi.top().num_troops * 2 << " troops were lost.\n";
                }

                generals[planets[planet_num].sith.top().general_id].lost_troops += planets[planet_num].jedi.top().num_troops;
                generals[planets[planet_num].jedi.top().general_id].lost_troops += planets[planet_num].jedi.top().num_troops;

                if(Opt.median) {

                    //when timestamp changes, print the median when median mode

                    if(planets[planet_num].s.size() == 0 && planets[planet_num].g.size() == 0) {
                        planets[planet_num].s.push(planets[planet_num].sith.top().num_troops * 2);
                        planets[planet_num].median = planets[planet_num].jedi.top().num_troops * 2;
                    }

                    else if (planets[planet_num].s.size() > planets[planet_num].g.size()) {

                        if (planets[planet_num].sith.top().num_troops * 2 < planets[planet_num].s.top()) {   
                            planets[planet_num].g.push(planets[planet_num].s.top());
                            planets[planet_num].s.pop();
                            planets[planet_num].s.push(planets[planet_num].sith.top().num_troops * 2);
                        }
                        else
                            planets[planet_num].g.push(planets[planet_num].sith.top().num_troops * 2);
            
                        planets[planet_num].median = (planets[planet_num].s.top() + planets[planet_num].g.top())/2;
                    }
                    else if (planets[planet_num].s.size() == planets[planet_num].g.size())
                    {
                        if (planets[planet_num].sith.top().num_troops * 2 < planets[planet_num].s.top()) {
                            planets[planet_num].s.push(planets[planet_num].sith.top().num_troops * 2);
                            planets[planet_num].median = planets[planet_num].s.top();
                        }
                        else {
                            planets[planet_num].g.push(planets[planet_num].sith.top().num_troops * 2);
                            planets[planet_num].median = planets[planet_num].g.top();
                        }
                    }
                    else {
                        if (planets[planet_num].sith.top().num_troops * 2 > planets[planet_num].g.top()) {
                            planets[planet_num].s.push(planets[planet_num].g.top());
                            planets[planet_num].g.pop();
                            planets[planet_num].g.push(planets[planet_num].sith.top().num_troops * 2);
                        }
                        else planets[planet_num].s.push(planets[planet_num].sith.top().num_troops * 2);
            
                        planets[planet_num].median = (planets[planet_num].s.top() + planets[planet_num].g.top())/2;
                    }

                }
                
                planets[planet_num].sith.pop();
                planets[planet_num].jedi.pop();
            }
        }
    } 
    
    if(Opt.median) {
            for(size_t j = 0 ; j < num_planets ; j ++) {
                if(planets[j].median != -1 && (planets[j].s.size() != 0 || planets[j].g.size() != 0)) {
                    cout << "Median troops lost on planet " << j << " at time " << previous << " is " << planets[j].median << ".\n";
                }
            }
    }

    cout << "---End of Day---\nBattles: " << battle_count << "\n";
    if(Opt.general_eval) {
        cout << "---General Evaluation---\n";
        for (size_t i = 0; i < generals.size(); ++i) {
            cout << "General " << i << " deployed " 
            << generals[i].jedi_total << " Jedi troops and " 
            << generals[i].sith_total << " Sith troops, and "  
            << generals[i].jedi_total + generals[i].sith_total - generals[i].lost_troops << "/" 
            << generals[i].jedi_total + generals[i].sith_total << " troops survived.\n";
        }
    }

    if(Opt.watcher) {
            
        cout << "---Movie Watcher---\n";

        // cout << planets[0].ambush_mode  << endl;
        // cout << planets[1].ambush_mode  << endl;


        for(size_t i = 0 ; i < num_planets ; i ++) {
            
            if(planets[i].ambush_mode == Seen_two || planets[i].ambush_mode == Maybe_better) {
                cout << "A movie watcher would enjoy an ambush on planet " << i 
                << " with Sith at time " << planets[i].best_ambush_sith.timestamp 
                << " and Jedi at time " << planets[i].best_ambush_jedi.timestamp 
                << " with a force difference of " << planets[i].best_ambush_sith.force_sens -  planets[i].best_ambush_jedi.force_sens << ".\n";
            }
            else {
                cout << "A movie watcher would not see an interesting ambush on planet " << i << ".\n";
            }

            // cout << "planets[1].attack_mode = " << planets[1].attack_mode << endl;
            // cout << planets[1].best_attack_sith.force_sens -  planets[1].best_attack_jedi.force_sens << endl;

            if(planets[i].attack_mode == Seen_two || planets[i].attack_mode == Maybe_better) {

                cout << "A movie watcher would enjoy an attack on planet " << i 
                << " with Jedi at time " << planets[i].best_attack_jedi.timestamp 
                << " and Sith at time " << planets[i].best_attack_sith.timestamp 
                << " with a force difference of " << planets[i].best_attack_sith.force_sens -  planets[i].best_attack_jedi.force_sens << ".\n";
            }
            else {
                cout << "A movie watcher would not see an interesting attack on planet " << i << ".\n";
            }
        }
    }


}

int main(int argc, char** argv){

    std::ios::sync_with_stdio(false);
    Stars stars;
    stars.get_options(argc, argv);
    stars.read_file();
}