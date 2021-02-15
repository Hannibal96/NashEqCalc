#include <iostream>
#include <tuple>
#include <fstream>
#include <string>
#include <unistd.h>
#include <map>
#include <algorithm>
#include <vector>

using namespace std;

/* Enums & typedefs:
 *      1. enum Scenario - list of all the AllIn/Fold combinations
 *      2. positions_expectancy - vector (of 4) doubles each represent the expected value of each positions
 *      3. positions_ranges - vector (of 4) ints each represent the range of the corresponding position
 *      4. position_strategy - vector of diffrent size represent the strategy of each position
 *      5. map_scenario_probability - map between ranges & scenario to the probability of that event happening
 *      6. map_ranges_equity - map between ranges and the equity of each one in case of action
 *      7. map_strategy_value - map between strategy and excpected value for specific position
 *      8. map_strategy_values - map between strategy and excpected value for all the positions
 *
 */

enum Scenario {empty_bigblind=0, oneraise_cutoff, oneraise_dealer, oneraise_smallblind,
    tworaises_cutoff_dealer, tworaises_cutoff_smallblind, tworaises_cutoff_bigblind, tworaises_dealer_smallblind,
    tworaises_dealer_bigblind, tworaises_smallblind_bigblind, threeraises_cutoff_dealer_smallblind,
    threeraises_cutoff_dealer_bigblind, threeraises_cutoff_smallblind_bigblind, threeraises_dealer_smallblind_bigblind,
    fourraises_cutoff_dealer_smallblind_bigblind        } ;

typedef vector<double> positions_expectancy;
typedef vector<int> positions_ranges;
typedef vector<int> position_strategy;
typedef map<tuple< positions_ranges, Scenario >, double > map_scenario_probability;
typedef map<positions_ranges, positions_expectancy > map_ranges_equity;
typedef map<position_strategy, double> map_strategy_value;
typedef map<position_strategy, positions_expectancy> map_strategy_values;

#define EQUITY_ERROR 0.15

/* Functions - done:
 *      1. string_to_scenario - convert string to the Scenario enum value
 *      2. convert the input to tuples in order to read the map_scenario_probability
 *      3. split string - cut string using delimiter and index
 *      4. get_ranges_equity -
 *      5. read_ranges_equity_file -
 *      6. read_scenario_probability_file -
 *      7. calc_iteration_value
 *      8. find_maximal_strategy
 *      9. calc_min_max
 *      10. calc_nash_definition
 *      11. init_ranges
 *      12. valid_params
 *      13. print_help
 *
 */

Scenario string_to_scenario(string& str);
double get_scenario_probability(map_scenario_probability & map, int co_range, int de_range, int sb_range, int bb_range, Scenario scenario);
string split_string(string & str, string & delimiter, int index);

positions_expectancy get_ranges_equity(map_ranges_equity & map, int co_range, int de_range, int sb_range, int bb_range);
map_ranges_equity read_ranges_equity_file();
map_scenario_probability read_scenario_probability_file();
positions_expectancy calc_iteration_value(double AllIn, double Bb, double Sb,
        int co_range, int de_range, int sb_range,
        int de_co_range, int sb_co_range, int sb_de_range, int bb_co_range, int bb_de_range, int bb_sb_range,
        int sb_co_de_range, int bb_co_de_range, int bb_co_sb_range, int bb_de_sb_range,
        int bb_co_de_sb_range,
        map_ranges_equity& ranges_equity_map, map_scenario_probability& scenario_probability_map) ;

position_strategy find_maximal_strategy(map_strategy_value);

vector<position_strategy> calc_min_max(map_ranges_equity & ranges_equity, map_scenario_probability & scenario_probability, double AllIn, double SmallBlind, double BigBlind,
        positions_ranges & co_range, positions_ranges & de_range, positions_ranges & de_co_range,
        positions_ranges & sb_range, positions_ranges & sb_co_range, positions_ranges & sb_de_range, positions_ranges & sb_co_de_range,
        positions_ranges & bb_co_range, positions_ranges & bb_de_range, positions_ranges & bb_sb_range,
        positions_ranges & bb_co_de_range, positions_ranges & bb_co_sb_range, positions_ranges & bb_de_sb_range, positions_ranges & bb_co_de_sb_range);

map_strategy_values calc_nash_definition(map_ranges_equity & ranges_equity, map_scenario_probability & scenario_probability,
        double AllIn, double SmallBlind, double BigBlind, double delta,
        positions_ranges & co_range, positions_ranges & de_range, positions_ranges & de_co_range,
        positions_ranges & sb_range, positions_ranges & sb_co_range, positions_ranges & sb_de_range, positions_ranges & sb_co_de_range,
        positions_ranges & bb_co_range, positions_ranges & bb_de_range, positions_ranges & bb_sb_range,
        positions_ranges & bb_co_de_range, positions_ranges & bb_co_sb_range, positions_ranges & bb_de_sb_range, positions_ranges & bb_co_de_sb_range);

void init_ranges(char *argv[], positions_ranges & co_range, positions_ranges & de_range, positions_ranges & de_co_range,
                 positions_ranges & sb_range, positions_ranges & sb_co_range, positions_ranges & sb_de_range, positions_ranges & sb_co_de_range,
                 positions_ranges & bb_co_range, positions_ranges & bb_de_range, positions_ranges & bb_sb_range,
                 positions_ranges & bb_co_de_range, positions_ranges & bb_co_sb_range, positions_ranges & bb_de_sb_range, positions_ranges & bb_co_de_sb_range);

bool valid_params(char *argv[], int argc);

void print_help();



/* ******************************************************************
 * ******************************************************************
 * Main
 * ******************************************************************
 * ******************************************************************
 */


int main(int argc, char *argv[]) {

    cout << "-I- Stating main..." << endl;

    if(!valid_params(argv, argc)){
        print_help();
        exit(1);
    }

    map_ranges_equity ranges_equity = read_ranges_equity_file();
    map_scenario_probability scenario_probability = read_scenario_probability_file();

    positions_ranges co_range, de_range, de_co_range, sb_range, sb_co_range, sb_de_range, sb_co_de_range,
            bb_co_range, bb_de_range, bb_sb_range, bb_co_de_range, bb_co_sb_range, bb_de_sb_range, bb_co_de_sb_range;

    string algo = argv[argc-1];

    init_ranges(argv, co_range, de_range, de_co_range, sb_range, sb_co_range, sb_de_range, sb_co_de_range, bb_co_range,
            bb_de_range, bb_sb_range, bb_co_de_range, bb_co_sb_range, bb_de_sb_range, bb_co_de_sb_range);


    double all_in=1.0, small_blind = 0.05, big_blind = 0.1;

    if(algo == "Nash" || algo == "NASH" || algo == "nash" ){
        double delta = 0.02;
        map_strategy_values nash_res = calc_nash_definition(ranges_equity, scenario_probability, all_in, small_blind, big_blind, delta,
                                                            co_range, de_range, de_co_range, sb_range, sb_co_range, sb_de_range, sb_co_de_range, bb_co_range, bb_de_range,
                                                            bb_sb_range, bb_co_de_range, bb_co_sb_range, bb_de_sb_range, bb_co_de_sb_range);
    }

    if(algo == "MinMax" || algo == "MINMAX" || algo == "minmax" ){
        vector<position_strategy> minmax_res = calc_min_max(ranges_equity, scenario_probability, all_in, small_blind, big_blind,
                                                            co_range, de_range, de_co_range, sb_range, sb_co_range, sb_de_range, sb_co_de_range, bb_co_range, bb_de_range,
                                                            bb_sb_range, bb_co_de_range, bb_co_sb_range, bb_de_sb_range, bb_co_de_sb_range);
    }


    /* Test data files
    positions_expectancy e = get_ranges_equity(ranges_equity, 50,5,25,10);
    for(int i=0; i<4; i++){
        cout << e[i] << " ";
    }
    cout << endl;
    for(int i=0; i<fourraises_cutoff_dealer_smallblind_bigblind+1;i++){
        cout << get_scenario_probability(scenario_probability, 5, 5, 5, 5, Scenario(i)) << endl;
    }
    */

    cout << "-I- Finishing main..." << endl;
    return 0;
}




/* *****************************************************
 * *****************************************************
 * Implementations:
 * *****************************************************
 * *****************************************************
 */

string split_string(string & str, string & delimiter, int index){
    string temp_string;
    size_t  last_match_index = 0;
    for(int i=0; i<index; i++){
        last_match_index = str.find(delimiter, last_match_index)+1;
    }
    size_t final_int = str.find(delimiter, last_match_index);
    return str.substr(last_match_index, final_int-last_match_index);
}

Scenario string_to_scenario(string & str){
    if(str == "empty_bigblind"){
        return empty_bigblind;
    }else if(str == "oneraise_cutoff"){
        return oneraise_cutoff;
    }else if(str == "oneraise_dealer"){
        return oneraise_dealer;
    }else if(str == "oneraise_smallblind"){
        return oneraise_smallblind;
    }else if(str == "tworaises_cutoff_dealer"){
        return tworaises_cutoff_dealer;
    }else if(str == "tworaises_cutoff_smallblind"){
        return tworaises_cutoff_smallblind;
    }else if(str == "tworaises_cutoff_bigblind"){
        return tworaises_cutoff_bigblind;
    }else if(str == "tworaises_dealer_smallblind"){
        return tworaises_dealer_smallblind;
    }else if(str == "tworaises_dealer_bigblind"){
        return tworaises_dealer_bigblind;
    }else if(str == "tworaises_smallblind_bigblind"){
        return tworaises_smallblind_bigblind;
    }else if(str == "threeraises_cutoff_dealer_smallblind"){
        return threeraises_cutoff_dealer_smallblind;
    }else if(str == "threeraises_cutoff_dealer_bigblind"){
        return threeraises_cutoff_dealer_bigblind;
    }else if(str == "threeraises_cutoff_smallblind_bigblind"){
        return threeraises_cutoff_smallblind_bigblind;
    }else if(str == "threeraises_dealer_smallblind_bigblind"){
        return threeraises_dealer_smallblind_bigblind;
    }else if(str == "fourraises_cutoff_dealer_smallblind_bigblind"){
        return fourraises_cutoff_dealer_smallblind_bigblind;
    }else{
        cout << "-E- invalid scenario" << endl;
        throw exception();
    }
}

double get_scenario_probability(map_scenario_probability &  map, int co_range, int de_range, int sb_range, int bb_range,
        Scenario scenario){
    return map[make_tuple(positions_ranges{co_range, de_range, sb_range, bb_range}, scenario)];
}

map_scenario_probability read_scenario_probability_file(){
    ifstream myfile;
    myfile.open ("./../frequency_dict_data.txt");
    if(myfile.is_open()){
        cout << "-I- frequency_dict_data file opened" << endl;
    } else{
        cout << "-E- failed to open frequency_dict_data file, Exiting..." << endl;
        throw exception();
    }

    string line;
    int index = 0;

    map_scenario_probability scenario_probability = map_scenario_probability();
    string delim_dot = ",", delim_open_brack = "(", delim_tag = "'", delim_dots = ":";

    int co_range, de_range, sb_range, bb_range;
    double probability;

    while ( getline (myfile, line) )
    {


        string temp, scenario;
        try {
            temp = split_string(line, delim_dot, 0);
            co_range = stoi(split_string(temp, delim_open_brack, 2)),
            de_range = stoi(split_string(line, delim_dot, 1)),
            sb_range = stoi(split_string(line, delim_dot, 2)),
            bb_range = stoi(split_string(line, delim_dot, 3));
            scenario = split_string(line, delim_tag, 1);
            probability = stod(split_string(line, delim_dots, 1));
        }
        catch (exception& e) {
            cout << endl << "-E- frequency_dict_data stoi error, line: " << index << endl;
            break;
        }
        index ++;
        if(index % 9830 == 0){
            cout << "=" << flush;
        }

        tuple<positions_ranges , Scenario > input_scenario =
                make_tuple(positions_ranges{co_range, de_range, sb_range, bb_range} , string_to_scenario(scenario));
        scenario_probability[input_scenario] = probability;
    }
    cout << endl;
    myfile.close();
    return scenario_probability;
}

map_ranges_equity read_ranges_equity_file(){
    ifstream myfile;
    myfile.open ("./../equity_dict_data.txt");
    if(myfile.is_open()){
        cout << "-I- equity_dict_data file opened" << endl;
    } else{
        cout << "-E- failed to open equity_dict_data file, Exiting..." << endl;
        throw exception();
    }

    map_ranges_equity ranges_equity_map = map_ranges_equity();
    string line;
    int index = 0;

    int co_range, de_range, sb_range, bb_range;
    double co_equity, de_equity, sb_equity, bb_equity;
    string delim_dots = ":", delim_comma = ",", delim_openbrac = "(", delim_closebrac = ")";

    while ( getline (myfile, line) ){

        try {
            string ranges_string = split_string(line, delim_dots, 0),
                    equity_string = split_string(line, delim_dots, 1);
            ranges_string = split_string(ranges_string, delim_openbrac, 1);
            ranges_string = split_string(ranges_string, delim_closebrac, 0);
            equity_string = split_string(equity_string, delim_openbrac, 1);
            equity_string = split_string(equity_string, delim_closebrac, 0);

            co_range = stoi(split_string(ranges_string, delim_comma, 0));
            de_range = stoi(split_string(ranges_string, delim_comma, 1));
            sb_range = stoi(split_string(ranges_string, delim_comma, 2));
            bb_range = stoi(split_string(ranges_string, delim_comma, 3));

            co_equity = stod(split_string(equity_string, delim_comma, 0));
            de_equity = stod(split_string(equity_string, delim_comma, 1));
            sb_equity = stod(split_string(equity_string, delim_comma, 2));
            bb_equity = stod(split_string(equity_string, delim_comma, 3));

        }
        catch (exception& e) {
            cout << endl << "-E- equity_dict_data stod error, line: " << index << endl;
            break;
        }

        index ++;

        positions_ranges ranges{co_range, de_range, sb_range, bb_range};
        positions_expectancy expectancies{co_equity, de_equity, sb_equity, bb_equity};

        ranges_equity_map[ranges] = expectancies;

        if(index % 18 == 0){
            cout << "=" << flush;
        }
    }
    cout << endl;

    return ranges_equity_map;

}

positions_expectancy get_ranges_equity(map_ranges_equity & map, int co_range, int de_range, int sb_range, int bb_range){

    vector<int> ranges{co_range, de_range, sb_range, bb_range};
    sort(ranges.begin(), ranges.end());

    if(ranges[2] == 0){
        return positions_expectancy{0,0,0,0};
    }

    auto itr_co = find(ranges.begin(), ranges.end(), co_range), itr_de = find(ranges.begin(), ranges.end(), de_range),
            itr_sb = find(ranges.begin(), ranges.end(), sb_range), itr_bb = find(ranges.begin(), ranges.end(), bb_range);

    positions_ranges sort_ranges{ranges[0], ranges[1], ranges[2], ranges[3]};

    const int index_co = distance(ranges.begin(), itr_co), index_de = distance(ranges.begin(), itr_de),
            index_sb = distance(ranges.begin(), itr_sb), index_bb = distance(ranges.begin(), itr_bb);

    positions_expectancy expectancy_unsort = map[sort_ranges];

    positions_expectancy expectancy_res =
            positions_expectancy{expectancy_unsort[index_co], expectancy_unsort[index_de], expectancy_unsort[index_sb], expectancy_unsort[index_bb]};

    if(abs(expectancy_res[0] + expectancy_res[1] + expectancy_res[2] + expectancy_res[3] - 100) > EQUITY_ERROR){
        cout << "-E- equity too divergent, total value: " ;
        cout << expectancy_res[0] + expectancy_res[1] + expectancy_res[2] + expectancy_res[3] << endl;
        cout << "    specific values: " << expectancy_res[0] << ", " << expectancy_res[1] << ", " <<
                expectancy_res[2] << ", " << expectancy_res[3] << endl;
        throw exception();
    }

    return expectancy_res;
}

void init_ranges(char *argv[], positions_ranges & co_range, positions_ranges & de_range, positions_ranges & de_co_range,
                 positions_ranges & sb_range, positions_ranges & sb_co_range, positions_ranges & sb_de_range, positions_ranges & sb_co_de_range,
                 positions_ranges & bb_co_range, positions_ranges & bb_de_range, positions_ranges & bb_sb_range,
                 positions_ranges & bb_co_de_range, positions_ranges & bb_co_sb_range, positions_ranges & bb_de_sb_range, positions_ranges & bb_co_de_sb_range){

    string pos = argv[1];
    if(pos == "de" || pos == "DE" || pos == "Dealer" || pos == "dealer"){
        co_range = positions_ranges{0};

        de_range = positions_ranges{5,10,15,20,25,30,35,40,45,50,60,70};
        de_co_range = positions_ranges{0};

        sb_range = positions_ranges{5,10,15,20,25,30,35,40,45,50,60,70};
        sb_co_range = positions_ranges{0};
        sb_de_range = positions_ranges{5,10,15,20,25,30,35,40,45,50,60,70};
        sb_co_de_range = positions_ranges{0};

        bb_co_range = positions_ranges{0};
        bb_de_range = positions_ranges{5,10,15,20,25,30,35,40,45,50,60,70};
        bb_sb_range = positions_ranges{5,10,15,20,25,30,35,40,45,50,60,70};
        bb_co_de_range = positions_ranges{0};
        bb_co_sb_range = positions_ranges{0};
        bb_de_sb_range = positions_ranges{5,10,15,20,25,30,35,40,45,50,60,70};
        bb_co_de_sb_range = positions_ranges{0};

    } else if(pos == "co" || pos == "CO" || pos == "CutOff" || pos == "cutoff" || pos == "Cutoff"){
        co_range = positions_ranges{15,20,25,30,35,40,45};

        de_range = positions_ranges{0};
        de_co_range = positions_ranges{5,10,15,20,25};

        sb_range = positions_ranges{0};
        sb_co_range = positions_ranges{5,10,15,20,25,30,35};
        sb_de_range = positions_ranges{0};
        sb_co_de_range = positions_ranges{5,10,15,20,25,30,35};

        bb_co_range = positions_ranges{5,10,15,20,25,30,35};
        bb_de_range = positions_ranges{0};
        bb_sb_range = positions_ranges{0};
        bb_co_de_range = positions_ranges{5,10,15,20,25,30,35};
        bb_co_sb_range = positions_ranges{5,10,15,20,25,30,35};
        bb_de_sb_range = positions_ranges{0};
        bb_co_de_sb_range = positions_ranges{5,10,15,20,25,30,35};
    } else if(pos == "sb" || pos == "SB" || pos == "SmallBlind" || pos == "smallblind" || pos == "Smallblind"){
        co_range = positions_ranges{0};

        de_range = positions_ranges{0};
        de_co_range = positions_ranges{0};

        sb_range = positions_ranges{5,10,15,20,25,30,35,40,45,50,60,70};
        sb_co_range = positions_ranges{0};
        sb_de_range = positions_ranges{0};
        sb_co_de_range = positions_ranges{0};

        bb_co_range = positions_ranges{0};
        bb_de_range = positions_ranges{0};
        bb_sb_range = positions_ranges{5,10,15,20,25,30,35,40,45,50,60,70};
        bb_co_de_range = positions_ranges{0};
        bb_co_sb_range = positions_ranges{0};
        bb_de_sb_range = positions_ranges{0};
        bb_co_de_sb_range = positions_ranges{0};
    }
    else{
        co_range = positions_ranges{20,25,30};

        de_range = positions_ranges{25,30,35};
        de_co_range = positions_ranges{5,10,15};

        sb_range = positions_ranges{50,60,70};
        sb_co_range = positions_ranges{5,10,15};
        sb_de_range = positions_ranges{10,15,20};
        sb_co_de_range = positions_ranges{5,10,15};

        bb_co_range = positions_ranges{20,25,30};
        bb_de_range = positions_ranges{20,25,30};
        bb_sb_range = positions_ranges{30,35,40};
        bb_co_de_range = positions_ranges{10,15,20};
        bb_co_sb_range = positions_ranges{10,15,20};
        bb_de_sb_range = positions_ranges{15,20,25};
        bb_co_de_sb_range = positions_ranges{5,10,15};
    }

}

bool valid_params(char *argv[], int argc){
    if(argc < 2 || argc > 3){
        return false;
    }

    string algo;
    if(argc == 2){
        algo = argv[1];
        if(algo == "MinMax" || algo == "MINMAX" || algo == "minmax" ||
           algo == "Nash" || algo == "NASH" || algo == "nash" ){
            return true;
        }
        return false;
    }

    string pos = argv[1];
    algo = argv[2];
    if(pos == "de" || pos == "DE" || pos == "Dealer" || pos == "dealer" ||
       pos == "sb" || pos == "SB" || pos == "SmallBlind" || pos == "smallblind" || pos == "Smallblind" ||
       pos == "co" || pos == "CO" || pos == "CutOff" || pos == "cutoff" || pos == "Cutoff"){

        if(algo == "MinMax" || algo == "MINMAX" || algo == "minmax" ||
           algo == "Nash" || algo == "NASH" || algo == "nash" ){
            return true;
        }
        return false;
    }

    return false;

}

void print_help(){
    cout << "--Help: keep format of <position> <algorithm> as input " << endl;
}

position_strategy find_maximal_strategy(map_strategy_value map){
    double max = -100;
    position_strategy max_strategy;
    for(auto const & strategy : map){
        if(strategy.second > max){
            max_strategy = strategy.first;
            max = strategy.second;
        }
    }
    return max_strategy;
}


vector<position_strategy> calc_min_max(map_ranges_equity & ranges_equity, map_scenario_probability & scenario_probability, double AllIn, double SmallBlind, double BigBlind,
        positions_ranges & co_range, positions_ranges & de_range, positions_ranges & de_co_range,
        positions_ranges & sb_range, positions_ranges & sb_co_range, positions_ranges & sb_de_range, positions_ranges & sb_co_de_range,
        positions_ranges & bb_co_range, positions_ranges & bb_de_range, positions_ranges & bb_sb_range,
        positions_ranges & bb_co_de_range, positions_ranges & bb_co_sb_range, positions_ranges & bb_de_sb_range, positions_ranges & bb_co_de_sb_range){

    map_strategy_value co_min_values = map_strategy_value(), de_min_values = map_strategy_value(), sb_min_values = map_strategy_value(), bb_min_values = map_strategy_value();
    unsigned int index = 0, total_iter = co_range.size() * de_range.size() * de_co_range.size() * sb_range.size() * sb_co_range.size() *
                                         sb_de_range.size() * sb_co_de_range.size() * bb_co_range.size() * bb_de_range.size() * bb_sb_range.size() *
                                         bb_co_de_range.size() * bb_co_sb_range.size() * bb_de_sb_range.size() * bb_co_de_sb_range.size();


    for(auto co_iter : co_range) {
        co_min_values[vector<int>{co_iter}] = 100;
    }
    for(auto de_iter : de_range) {
        for (auto de_co_iter : de_co_range) {
            de_min_values[vector<int>{de_iter, de_co_iter}] = 100;
        }
    }
    for (auto sb_iter : sb_range) {
        for (auto sb_co_iter : sb_co_range) {
            for (auto sb_de_iter : sb_de_range) {
                for (auto sb_co_de_iter : sb_co_de_range) {
                    sb_min_values[vector<int>{sb_iter, sb_co_iter, sb_de_iter, sb_co_de_iter}] = 100;
                }
            }
        }
    }
    for (auto bb_co_iter : bb_co_range) {
        for (auto bb_de_iter : bb_de_range) {
            for (auto bb_sb_iter : bb_sb_range) {
                for (auto bb_co_de_iter : bb_co_de_range) {
                    for (auto bb_co_sb_iter : bb_co_sb_range) {
                        for (auto bb_de_sb_iter : bb_de_sb_range) {
                            for (auto bb_co_de_sb_iter : bb_co_de_sb_range) {
                                bb_min_values[vector<int>{bb_co_iter, bb_de_iter, bb_sb_iter,
                                                          bb_co_de_iter, bb_co_sb_iter, bb_de_sb_iter, bb_co_de_sb_iter}] = 100;
                            }
                        }
                    }
                }
            }
        }
    }

    cout << "-I- Starting calculation of min max algorithm..." << endl;
    for(auto co_iter : co_range){
        /*
         * */
        for(auto de_iter : de_range) {
            for (auto de_co_iter : de_co_range) {
                /*
                * */
                for (auto sb_iter : sb_range) {
                    for (auto sb_co_iter : sb_co_range) {
                        for (auto sb_de_iter : sb_de_range) {
                            for (auto sb_co_de_iter : sb_co_de_range) {
                                /*
                                * */
                                for (auto bb_co_iter : bb_co_range) {
                                    for (auto bb_de_iter : bb_de_range) {
                                        for (auto bb_sb_iter : bb_sb_range) {
                                            for (auto bb_co_de_iter : bb_co_de_range) {
                                                for (auto bb_co_sb_iter : bb_co_sb_range) {
                                                    for (auto bb_de_sb_iter : bb_de_sb_range) {
                                                        for (auto bb_co_de_sb_iter : bb_co_de_sb_range) {
                                                            /*
                                                            * */
                                                            positions_expectancy e =
                                                                    calc_iteration_value(AllIn, BigBlind, SmallBlind,
                                                                                         co_iter, de_iter, sb_iter, de_co_iter, sb_co_iter, sb_de_iter,
                                                                                         bb_co_iter, bb_de_iter, bb_sb_iter, sb_co_de_iter, bb_co_de_iter,
                                                                                         bb_co_sb_iter, bb_de_sb_iter, bb_co_de_sb_iter,
                                                                                         ranges_equity, scenario_probability);

                                                            if(e[0] < co_min_values[vector<int>{co_iter}]){
                                                                co_min_values[vector<int>{co_iter}] = e[0];
                                                            }
                                                            if(e[1] < de_min_values[vector<int>{de_iter, de_co_iter}] ){
                                                                de_min_values[vector<int>{de_iter, de_co_iter}] = e[1];
                                                            }
                                                            if(e[2] < sb_min_values[vector<int>{sb_iter, sb_co_iter, sb_de_iter, sb_co_de_iter}]){
                                                                sb_min_values[vector<int>{sb_iter, sb_co_iter, sb_de_iter, sb_co_de_iter}] = e[2];
                                                            }
                                                            if(e[3] < bb_min_values[vector<int>{bb_co_iter, bb_de_iter, bb_sb_iter,
                                                                                                bb_co_de_iter, bb_co_sb_iter, bb_de_sb_iter, bb_co_de_sb_iter}]){
                                                                bb_min_values[vector<int>{bb_co_iter, bb_de_iter, bb_sb_iter,
                                                                                          bb_co_de_iter, bb_co_sb_iter, bb_de_sb_iter, bb_co_de_sb_iter}] = e[3];
                                                            }

                                                            index ++;
                                                            if(index % (total_iter/100 + !(total_iter/100)) == 0){
                                                                cout << "=" << flush;
                                                            }

                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    cout << endl;

    position_strategy max_co = find_maximal_strategy(co_min_values);
    cout << "CO result: " << endl;
    for(auto i : max_co)
        cout << i << ", ";
    cout << endl;
    cout << co_min_values[max_co] << endl;

    position_strategy max_de = find_maximal_strategy(de_min_values);
    cout << "DE result: " << endl;
    for(auto i : max_de)
        cout << i << ", ";
    cout << endl;
    cout << de_min_values[max_de] << endl;

    position_strategy max_sb = find_maximal_strategy(sb_min_values);
    cout << "SB result: " << endl;
    for(auto i : max_sb)
        cout << i << ", ";
    cout << endl;
    cout << sb_min_values[max_sb] << endl;

    position_strategy max_bb = find_maximal_strategy(bb_min_values);
    cout << "BB result: " << endl;
    for(auto i : max_bb)
        cout << i << ", ";
    cout << endl;
    cout << bb_min_values[max_bb] << endl;

    return vector<position_strategy>{max_co, max_de, max_sb, max_bb};
}



map_strategy_values calc_nash_definition(map_ranges_equity & ranges_equity, map_scenario_probability & scenario_probability,
        double AllIn, double SmallBlind, double BigBlind, double delta,
        positions_ranges & co_range, positions_ranges & de_range, positions_ranges & de_co_range,
        positions_ranges & sb_range, positions_ranges & sb_co_range, positions_ranges & sb_de_range, positions_ranges & sb_co_de_range,
        positions_ranges & bb_co_range, positions_ranges & bb_de_range, positions_ranges & bb_sb_range,
        positions_ranges & bb_co_de_range, positions_ranges & bb_co_sb_range, positions_ranges & bb_de_sb_range, positions_ranges & bb_co_de_sb_range){


    map_strategy_values nash_points_values = map_strategy_values();
    unsigned int index = 0, total_iter = co_range.size() * de_range.size() * de_co_range.size() * sb_range.size() * sb_co_range.size() *
                                         sb_de_range.size() * sb_co_de_range.size() * bb_co_range.size() * bb_de_range.size() * bb_sb_range.size() *
                                         bb_co_de_range.size() * bb_co_sb_range.size() * bb_de_sb_range.size() * bb_co_de_sb_range.size();

    cout << "-I- Starting calculation of nash point by definition..." << endl;

    double margin = delta;
    while(nash_points_values.empty()) {
        cout << "-I- Current margin: " << margin << endl;

        for (auto co_iter : co_range) {
            /*         *
             */
            for (auto de_iter : de_range) {
                for (auto de_co_iter : de_co_range) {
                    /*
                    */
                    for (auto sb_iter : sb_range) {
                        for (auto sb_co_iter : sb_co_range) {
                            for (auto sb_de_iter : sb_de_range) {
                                for (auto sb_co_de_iter : sb_co_de_range) {
                                    /*
                                     */
                                    for (auto bb_co_iter : bb_co_range) {
                                        for (auto bb_de_iter : bb_de_range) {
                                            for (auto bb_sb_iter : bb_sb_range) {
                                                for (auto bb_co_de_iter : bb_co_de_range) {
                                                    for (auto bb_co_sb_iter : bb_co_sb_range) {
                                                        for (auto bb_de_sb_iter : bb_de_sb_range) {
                                                            for (auto bb_co_de_sb_iter : bb_co_de_sb_range) {

                                                                positions_expectancy e =
                                                                        calc_iteration_value(AllIn, BigBlind, SmallBlind,
                                                                                             co_iter, de_iter, sb_iter,
                                                                                             de_co_iter, sb_co_iter, sb_de_iter,
                                                                                             bb_co_iter, bb_de_iter,bb_sb_iter,
                                                                                             sb_co_de_iter,bb_co_de_iter,
                                                                                             bb_co_sb_iter,bb_de_sb_iter,bb_co_de_sb_iter,
                                                                                             ranges_equity,scenario_probability);
                                                                bool is_nash = true;

                                                                for (auto temp_co_iter : co_range) {
                                                                    positions_expectancy t =
                                                                            calc_iteration_value(AllIn, BigBlind,SmallBlind,
                                                                                                 temp_co_iter, de_iter,sb_iter,
                                                                                                 de_co_iter,sb_co_iter, sb_de_iter,
                                                                                                 bb_co_iter, bb_de_iter,bb_sb_iter,
                                                                                                 sb_co_de_iter,bb_co_de_iter,bb_co_sb_iter,
                                                                                                 bb_de_sb_iter,bb_co_de_sb_iter,
                                                                                                 ranges_equity,scenario_probability);

                                                                    if (t[0] > e[0] + margin * abs(e[0])) {
                                                                        is_nash = false;
                                                                        break;
                                                                    }
                                                                }
                                                                if (not is_nash) {continue;}

                                                                for (auto temp_de_iter : de_range) {
                                                                    for (auto temp_de_co_iter : de_co_range) {
                                                                        positions_expectancy t =
                                                                                calc_iteration_value(AllIn, BigBlind,SmallBlind,
                                                                                        co_iter,temp_de_iter,sb_iter,temp_de_co_iter,
                                                                                        sb_co_iter,sb_de_iter,bb_co_iter,bb_de_iter,
                                                                                        bb_sb_iter,sb_co_de_iter,bb_co_de_iter,
                                                                                        bb_co_sb_iter,bb_de_sb_iter,bb_co_de_sb_iter,
                                                                                        ranges_equity,scenario_probability);

                                                                        if (t[1] > e[1] + margin * abs(e[1])) {
                                                                            is_nash = false;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                                if (not is_nash) {continue;}

                                                                for (auto temp_sb_iter : sb_range) {
                                                                    for (auto temp_sb_co_iter : sb_co_range) {
                                                                        for (auto temp_sb_de_iter : sb_de_range) {
                                                                            for (auto temp_sb_co_de_co_iter : sb_co_de_range) {
                                                                                positions_expectancy t =
                                                                                        calc_iteration_value(
                                                                                                AllIn, BigBlind,SmallBlind,
                                                                                                co_iter, de_iter,temp_sb_iter,
                                                                                                de_co_iter,temp_sb_co_iter,temp_sb_de_iter,
                                                                                                bb_co_iter, bb_de_iter,bb_sb_iter,
                                                                                                temp_sb_co_de_co_iter,bb_co_de_iter,bb_co_sb_iter,
                                                                                                bb_de_sb_iter,bb_co_de_sb_iter,
                                                                                                ranges_equity,scenario_probability);
                                                                                if (t[2] > e[2] + margin * abs(e[2])) {
                                                                                    is_nash = false;
                                                                                    break;
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                                if (not is_nash) {continue;}

                                                                for (auto temp_bb_co_iter : bb_co_range) {
                                                                    for (auto temp_bb_de_iter : bb_de_range) {
                                                                        for (auto temp_bb_sb_iter : bb_sb_range) {
                                                                            for (auto temp_bb_co_de_iter : bb_co_de_range) {
                                                                                for (auto temp_bb_co_sb_iter : bb_co_sb_range) {
                                                                                    for (auto temp_bb_de_sb_iter : bb_de_sb_range) {
                                                                                        for (auto temp_bb_co_de_sb_iter : bb_co_de_sb_range) {
                                                                                            positions_expectancy t =
                                                                                                    calc_iteration_value(
                                                                                                       AllIn,BigBlind,SmallBlind,
                                                                                                       co_iter,de_iter,sb_iter,de_co_iter,
                                                                                                       sb_co_iter,sb_de_iter,temp_bb_co_iter,
                                                                                                       temp_bb_de_iter,temp_bb_sb_iter,
                                                                                                       sb_co_de_iter,temp_bb_co_de_iter,
                                                                                                       temp_bb_co_sb_iter,temp_bb_de_sb_iter,
                                                                                                       temp_bb_co_de_sb_iter,
                                                                                                       ranges_equity,scenario_probability);

                                                                                            if (t[3] > e[3] + margin * abs(e[3])) {
                                                                                                is_nash = false;
                                                                                                break;
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }

                                                                if (is_nash) {
                                                                    nash_points_values[position_strategy{co_iter, de_iter, de_co_iter,
                                                                           sb_iter, sb_co_iter, sb_de_iter, sb_co_de_iter,
                                                                           bb_co_iter, bb_de_iter, bb_sb_iter,
                                                                           bb_co_de_iter, bb_co_sb_iter, bb_de_sb_iter,
                                                                           bb_co_de_sb_iter}] = e;
                                                                }

                                                            }

                                                            index ++;
                                                            if(index % (total_iter/100 + !(total_iter/100)) == 0){
                                                                cout << "=" << flush;
                                                            }

                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        cout << endl;
        margin += delta;
    }

    cout << "-I- Results:" << endl;
    for(auto x: nash_points_values){
        cout << "CO: "<< x.first[0] << endl << x.second[0] << endl
        << "DE: " << x.first[1] << ", " << x.first[2] << endl << x.second[1] << endl
        << "SB: " << x.first[3] << ", " << x.first[4] << ", " << x.first[5] << ", " << x.first[6] << endl << x.second[2] << endl
        << "BB: " << x.first[7] << ", " << x.first[8] << ", " << x.first[9] << ", " << x.first[10] << ", "
            << x.first[11] << ", " << x.first[12] << ", " << x.first[13] << endl << x.second[3] << endl;
    }

    return nash_points_values;
}



positions_expectancy calc_iteration_value(double AllIn, double Bb, double Sb,
              int co_range, int de_range, int sb_range, int de_co_range, int sb_co_range, int sb_de_range,
              int bb_co_range, int bb_de_range, int bb_sb_range, int sb_co_de_range, int bb_co_de_range,
              int bb_co_sb_range, int bb_de_sb_range, int bb_co_de_sb_range,
              map_ranges_equity& ranges_equity_map, map_scenario_probability& scenario_probability_map) {

    double  probability_empty_bigblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_range, sb_range, 0, empty_bigblind),
            probability_oneraise_cutoff = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_co_range, sb_co_range, bb_co_range, oneraise_cutoff),
            probability_probability_oneraise_dealer = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_range, sb_de_range, bb_de_range, oneraise_dealer),
            probability_oneraise_smallblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_range, sb_range, bb_sb_range, oneraise_smallblind),
            probability_tworaises_cutoff_dealer = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_co_range, sb_co_de_range, bb_co_de_range, tworaises_cutoff_dealer),
            probability_tworaises_cutoff_smallblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_co_range, sb_co_range, bb_co_sb_range, tworaises_cutoff_smallblind),
            probability_tworaises_cutoff_bigblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_co_range, sb_co_range, bb_co_range, tworaises_cutoff_bigblind),
            probability_tworaises_dealer_smallblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_range, sb_de_range, bb_de_sb_range, tworaises_dealer_smallblind),
            probability_tworaises_dealer_bigblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_range, sb_de_range, bb_de_range, tworaises_dealer_bigblind),
            probability_probability_tworaises_smallblind_bigblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_range, sb_range, bb_sb_range, tworaises_smallblind_bigblind),
            probability_threeraises_cutoff_dealer_smallblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_co_range, sb_co_de_range, bb_co_de_sb_range, threeraises_cutoff_dealer_smallblind),
            probability_threeraises_cutoff_dealer_bigblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_co_range, sb_co_de_range, bb_co_de_range, threeraises_cutoff_dealer_bigblind),
            probability_threeraises_cutoff_smallblind_bigblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_co_range, sb_co_range, bb_co_sb_range, threeraises_cutoff_smallblind_bigblind),
            probability_threeraises_dealer_smallblind_bigblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_range, sb_de_range, bb_de_sb_range, threeraises_dealer_smallblind_bigblind),
            probability_fourraises_cutoff_dealer_smallblind_bigblind = (0.01) * get_scenario_probability(scenario_probability_map, co_range, de_co_range, sb_co_de_range, bb_co_de_sb_range, fourraises_cutoff_dealer_smallblind_bigblind);

    if(abs(probability_empty_bigblind + probability_oneraise_cutoff + probability_probability_oneraise_dealer + probability_oneraise_smallblind
       +probability_tworaises_cutoff_dealer + probability_tworaises_cutoff_smallblind + probability_tworaises_cutoff_bigblind + probability_tworaises_dealer_smallblind
       + probability_tworaises_dealer_bigblind + probability_probability_tworaises_smallblind_bigblind + probability_threeraises_cutoff_dealer_smallblind +
       probability_threeraises_cutoff_dealer_bigblind + probability_threeraises_cutoff_smallblind_bigblind + probability_threeraises_dealer_smallblind_bigblind+
       probability_fourraises_cutoff_dealer_smallblind_bigblind - 1.0) > 0.0015)
    {
        cout << "-E- Scenario probability too divergent, total value: " ;
        cout << (probability_empty_bigblind + probability_oneraise_cutoff + probability_probability_oneraise_dealer + probability_oneraise_smallblind
                 +probability_tworaises_cutoff_dealer + probability_tworaises_cutoff_smallblind + probability_tworaises_cutoff_bigblind + probability_tworaises_dealer_smallblind
                 + probability_tworaises_dealer_bigblind + probability_probability_tworaises_smallblind_bigblind + probability_threeraises_cutoff_dealer_smallblind +
                 probability_threeraises_cutoff_dealer_bigblind + probability_threeraises_cutoff_smallblind_bigblind + probability_threeraises_dealer_smallblind_bigblind+
                 probability_fourraises_cutoff_dealer_smallblind_bigblind ) << endl;
        throw exception();
    }

    vector<double > co_VS_de_equity = get_ranges_equity(ranges_equity_map, 0,0,co_range,de_co_range),
                    co_VS_sb_equity = get_ranges_equity(ranges_equity_map, 0,0,co_range,sb_co_range),
                    co_VS_bb_equity = get_ranges_equity(ranges_equity_map, 0,0,co_range,bb_co_range),
                    de_VS_sb_equity = get_ranges_equity(ranges_equity_map, 0,0,de_range,sb_de_range),
                    de_VS_bb_equity = get_ranges_equity(ranges_equity_map, 0,0,de_range,bb_de_range),
                    sb_VS_bb_equity = get_ranges_equity(ranges_equity_map, 0,0,sb_range,bb_sb_range),
                    co_VS_de_VS_sb_equity = get_ranges_equity(ranges_equity_map, 0,co_range,de_co_range,sb_co_de_range),
                    co_VS_de_VS_bb_equity = get_ranges_equity(ranges_equity_map, 0,co_range,de_co_range,bb_co_de_range),
                    co_VS_sb_VS_bb_equity = get_ranges_equity(ranges_equity_map, 0,co_range,sb_co_range,bb_co_sb_range),
                    de_VS_sb_VS_bb_equity = get_ranges_equity(ranges_equity_map, 0,de_range,sb_de_range,bb_de_sb_range),
                    co_VS_de_VS_sb_VS_bb_equity = get_ranges_equity(ranges_equity_map, co_range,de_co_range,sb_co_de_range,bb_co_de_sb_range);

    double co_value =
            probability_empty_bigblind                               * 1                                       * 0                   +
            probability_oneraise_cutoff                              * 1                                       * (Sb + Bb)           +
            probability_probability_oneraise_dealer                  * 1                                       * 0                   +
            probability_oneraise_smallblind                          * 1                                       * 0                   +
            probability_tworaises_cutoff_dealer                      * ((0.01) * co_VS_de_equity[2]             * (2*AllIn + Sb + Bb) - AllIn) +
            probability_tworaises_cutoff_smallblind                  * ((0.01) * co_VS_sb_equity[2]             * (2*AllIn + Bb) - AllIn)      +
            probability_tworaises_cutoff_bigblind                    * ((0.01) * co_VS_bb_equity[2]             * (2*AllIn + Sb) - AllIn)      +
            probability_tworaises_dealer_smallblind                  * 1                                       * 0                   +
            probability_tworaises_dealer_bigblind                    * 1                                       * 0                   +
            probability_probability_tworaises_smallblind_bigblind    * 1                                       * 0                   +
            probability_threeraises_cutoff_dealer_smallblind         * ((0.01) * co_VS_de_VS_sb_equity[1]       * (3*AllIn + Bb) - AllIn)      +
            probability_threeraises_cutoff_dealer_bigblind           * ((0.01) * co_VS_de_VS_bb_equity[1]       * (3*AllIn + Sb) - AllIn)     +
            probability_threeraises_cutoff_smallblind_bigblind       * ((0.01) * co_VS_sb_VS_bb_equity[1]       * (3*AllIn) - AllIn)          +
            probability_threeraises_dealer_smallblind_bigblind       * 1                                       * 0                   +
            probability_fourraises_cutoff_dealer_smallblind_bigblind * ((0.01) * co_VS_de_VS_sb_VS_bb_equity[0] * (4*AllIn) - AllIn);

    double de_value =
            probability_empty_bigblind                               * 1                                       * 0                   +
            probability_oneraise_cutoff                              * 1                                       * 0                   +
            probability_probability_oneraise_dealer                  * 1                                       * (Sb + Bb)           +
            probability_oneraise_smallblind                          * 1                                       * 0                   +
            probability_tworaises_cutoff_dealer                      * ((0.01) * co_VS_de_equity[3]             * (2*AllIn + Sb + Bb) - AllIn) +
            probability_tworaises_cutoff_smallblind                  * 1                                       * 0                   +
            probability_tworaises_cutoff_bigblind                    * 1                                       * 0                   +
            probability_tworaises_dealer_smallblind                  * ((0.01) * de_VS_sb_equity[2]             * (2*AllIn + Bb) - AllIn)     +
            probability_tworaises_dealer_bigblind                    * ((0.01) * de_VS_bb_equity[2]             * (2*AllIn + Sb) - AllIn)     +
            probability_probability_tworaises_smallblind_bigblind    * 1                                       * 0                   +
            probability_threeraises_cutoff_dealer_smallblind         * ((0.01) * co_VS_de_VS_sb_equity[2]       * (3*AllIn + Bb) - AllIn)      +
            probability_threeraises_cutoff_dealer_bigblind           * ((0.01) * co_VS_de_VS_bb_equity[2]       * (3*AllIn + Sb) - AllIn)     +
            probability_threeraises_cutoff_smallblind_bigblind       * 1                                       * 0                   +
            probability_threeraises_dealer_smallblind_bigblind       * ((0.01) * de_VS_sb_VS_bb_equity[1]       * (3*AllIn) - AllIn)          +
            probability_fourraises_cutoff_dealer_smallblind_bigblind * ((0.01) * co_VS_de_VS_sb_VS_bb_equity[1] * (4*AllIn) - AllIn);

    double sb_value =
            probability_empty_bigblind                               * 1                                       * (-Sb)          +
            probability_oneraise_cutoff                              * 1                                       * (-Sb)          +
            probability_probability_oneraise_dealer                  * 1                                       * (-Sb)          +
            probability_oneraise_smallblind                          * 1                                       * (+Bb)          +
            probability_tworaises_cutoff_dealer                      * 1                                       * (-Sb)          +
            probability_tworaises_cutoff_smallblind                  * ((0.01) * co_VS_sb_equity[3]             * (2*AllIn + Bb)  - AllIn)+
            probability_tworaises_cutoff_bigblind                    * 1                                       * (-Sb)          +
            probability_tworaises_dealer_smallblind                  * ((0.01) * de_VS_sb_equity[3]             * (2*AllIn + Bb)  - AllIn)+
            probability_tworaises_dealer_bigblind                    * 1                                       * (-Sb)          +
            probability_probability_tworaises_smallblind_bigblind    * ((0.01) * sb_VS_bb_equity[2]             * (2*AllIn)      - AllIn)+
            probability_threeraises_cutoff_dealer_smallblind         * ((0.01) * co_VS_de_VS_sb_equity[3]       * (3*AllIn + Bb) - AllIn)+
            probability_threeraises_cutoff_dealer_bigblind           * 1                                       * (-Sb)          +
            probability_threeraises_cutoff_smallblind_bigblind       * ((0.01) * co_VS_sb_VS_bb_equity[2]       * (3*AllIn)      -AllIn) +
            probability_threeraises_dealer_smallblind_bigblind       * ((0.01) * de_VS_sb_VS_bb_equity[2]       * (3*AllIn)      -AllIn)+
            probability_fourraises_cutoff_dealer_smallblind_bigblind * ((0.01) * co_VS_de_VS_sb_VS_bb_equity[2] * (4*AllIn) - AllIn);

    double bb_value =
            probability_empty_bigblind                               * 1                                       * (Sb)           +
            probability_oneraise_cutoff                              * 1                                       * (-Bb)          +
            probability_probability_oneraise_dealer                  * 1                                       * (-Bb)          +
            probability_oneraise_smallblind                          * 1                                       * (-Bb)          +
            probability_tworaises_cutoff_dealer                      * 1                                       * (-Bb)          +
            probability_tworaises_cutoff_smallblind                  * 1                                       * (-Bb)          +
            probability_tworaises_cutoff_bigblind                    * ((0.01) * co_VS_bb_equity[3]             * (2*AllIn + Sb) -AllIn)+
            probability_tworaises_dealer_smallblind                  * 1                                       * (-Bb)          +
            probability_tworaises_dealer_bigblind                    * ((0.01) * de_VS_bb_equity[3]             * (2*AllIn + Sb) -AllIn)+
            probability_probability_tworaises_smallblind_bigblind    * ((0.01) * sb_VS_bb_equity[3]             * (2*AllIn)      -AllIn)+
            probability_threeraises_cutoff_dealer_smallblind         * 1                                       * (-Bb)          +
            probability_threeraises_cutoff_dealer_bigblind           * ((0.01) * co_VS_de_VS_bb_equity[3]       * (3*AllIn + Sb) -AllIn)+
            probability_threeraises_cutoff_smallblind_bigblind       * ((0.01) * co_VS_sb_VS_bb_equity[3]       * (3*AllIn)      -AllIn)+
            probability_threeraises_dealer_smallblind_bigblind       * ((0.01) * de_VS_sb_VS_bb_equity[3]       * (3*AllIn)      -AllIn)+
            probability_fourraises_cutoff_dealer_smallblind_bigblind * ((0.01) * co_VS_de_VS_sb_VS_bb_equity[3] * (4*AllIn) -AllIn);

    double value_error = (Sb+!Sb)/10;
    if(abs(co_value+de_value+sb_value+bb_value) > value_error){
        cout << "-E- value_error too big, total value: " ;
        cout << co_value + de_value + sb_value + bb_value << endl;
        cout << co_value << "," << de_value << "," << sb_value << "," << bb_value << endl;
        throw exception();
    }

    positions_expectancy iter_value{co_value, de_value, sb_value, bb_value};

    return iter_value;
}

/*
 * input: AllIn, Bb, Sb,
 *        co_range, de_range, sb_range,
 *        de_co_range, sb_co_range, sb_de_range, bb_co_range, bb_de_range, bb_sb_range,
 *        sb_co_de_range, bb_co_de_range, bb_co_sb_range, bb_de_sb_range,
 *        bb_co_de_sb_range
 *
 * CO = P{CO_not, DE_not, SB_not}                   * E{1} * 0                                                                  +       (empty)
 *      P{CO, DE_CO_not, SB_CO_not, BB_CO_not}      * E{1} * Bb+Sb                                                              +       (only co)
 *      P{CO_not, DE, SB_DE_not, BB_DE_not}         * E{1} * 0                                                                  +       (only de)
 *      P{CO_not, DE_not, SB, BB_SB_not}            * E{1} * 0                                                                  +       (only sb)
 *      P{CO, DE_CO, SB_CO_DE_not, BB_CO_DE_not}    * E{co_range, de_co_range} * (2*AllIn + Bb + Sb)                            +       (co vs de)
 *      P{CO, DE_CO_not, SB_CO, BB_CO_SB_not}       * E{co_range, sb_co_range} * (2*AllIn + Bb)                                 +       (co vs sb)
 *      P{CO, DE_CO_not, SB_CO_not, BB_CO}          * E{co_range, bb_co_range} * (2*AllIn + Sb)                                 +       (co vs bb)
 *      P{CO_not, DE, SB_DE, BB_DE_SB_not}          * E{1} * 0                                                                  +       (de vs sb)
 *      P{CO_not, DE, SB_DE_not, BB_DE}             * E{1} * 0                                                                  +       (de vs bb)
 *      P{CO_not, DE_not, SB, BB_SB}                * E{1} * 0                                                                  +       (sb vs bb)
 *      P{CO, DE_CO, SB_CO_DE, BB_CO_DE_SB_not}     * E{co_range, de_co_range, sb_co_de_range} * (3*AllIn + Bb)                 +       (co vs de vs sb)
 *      P{CO, DE_CO, SB_CO_DE_not, BB_CO_DE}        * E{co_range, de_co_range, bb_co_de_range} * (3*AllIn + Sb)                 +       (co vs de vs bb)
 *      P{CO, DE_CO_not, SB_CO, BB_CO_SB}           * E{co_range, sb_co_range, bb_co_sb_range} * (3*AllIn)                      +       (co vs sb vs bb)
 *      P{CO_not, DE, SB_DE, BB_DE_SB}              * E{1} * 0                                                                  +       (de vs sb vs bb)
 *      P{CO, DE_CO, SB_CO_DE, BB_CO_DE_SB}         * E{co_range, de_co_range, sb_co_de_range, bb_co_de_sb_range} * (4*AllIn)   +       (co vs de vs sb vs bb)
 *
 *
 * DE = P{CO_not, DE_not, SB_not}                   * E{1} * 0                      +       (empty)
 *      P{CO, DE_CO_not, SB_CO_not, BB_CO_not}      * E{1} * 0                      +       (only co)
 *      P{CO_not, DE, SB_DE_not, BB_DE_not}         * E{1} * Bb+Sb                  +       (only de)
 *      P{CO_not, DE_not, SB, BB_SB_not}            * E{1} * 0                      +       (only sb)
 *      P{CO, DE_CO, SB_CO_DE_not, BB_CO_DE_not}    * E{} * X                       +       (co vs de)
 *      P{CO, DE_CO_not, SB_CO, BB_CO_SB_not}       * E{1} * 0                      +       (co vs sb)
 *      P{CO, DE_CO_not, SB_CO_not, BB_CO}          * E{1} * 0                      +       (co vs bb)
 *      P{CO_not, DE, SB_DE, BB_DE_SB_not}          * E{} * X                       +       (de vs sb)
 *      P{CO_not, DE, SB_DE_not, BB_DE}             * E{} * X                       +       (de vs bb)
 *      P{CO_not, DE_not, SB, BB_SB}                * E{1} * 0                      +       (sb vs bb)
 *      P{CO, DE_CO, SB_CO_DE, BB_CO_DE_SB_not}     * E{} * X                       +       (co vs de vs sb)
 *      P{CO, DE_CO, SB_CO_DE_not, BB_CO_DE}        * E{} * X                       +       (co vs de vs bb)
 *      P{CO, DE_CO_not, SB_CO, BB_CO_SB}           * E{1} * 0                      +       (co vs sb vs bb)
 *      P{CO_not, DE, SB_DE, BB_DE_SB}              * E{} * X                       +       (de vs sb vs bb)
 *      P{CO, DE_CO, SB_CO_DE, BB_CO_DE_SB}         * E{} * X                       +       (co vs de vs sb vs bb)
 *
 *
 * SB = P{CO_not, DE_not, SB_not}                   * E{1} * (-Sb)                  +       (empty)
 *      P{CO, DE_CO_not, SB_CO_not, BB_CO_not}      * E{1} * (-Sb)                  +       (only co)
 *      P{CO_not, DE, SB_DE_not, BB_DE_not}         * E{1} * (-Sb)                  +       (only de)
 *      P{CO_not, DE_not, SB, BB_SB_not}            * E{1} * Bb                     +       (only sb)
 *      P{CO, DE_CO, SB_CO_DE_not, BB_CO_DE_not}    * E{1} * (-Sb)                  +       (co vs de)
 *      P{CO, DE_CO_not, SB_CO, BB_CO_SB_not}       * E{} * X                       +       (co vs sb)
 *      P{CO, DE_CO_not, SB_CO_not, BB_CO}          * E{1} * (-Sb)                  +       (co vs bb)
 *      P{CO_not, DE, SB_DE, BB_DE_SB_not}          * E{} * X                       +       (de vs sb)
 *      P{CO_not, DE, SB_DE_not, BB_DE}             * E{1} * (-Sb)                  +       (de vs bb)
 *      P{CO_not, DE_not, SB, BB_SB}                * E{} * X                       +       (sb vs bb)
 *      P{CO, DE_CO, SB_CO_DE, BB_CO_DE_SB_not}     * E{} * X                       +       (co vs de vs sb)
 *      P{CO, DE_CO, SB_CO_DE_not, BB_CO_DE}        * E{1} * (-Sb)                  +       (co vs de vs bb)
 *      P{CO, DE_CO_not, SB_CO, BB_CO_SB}           * E{} * X                       +       (co vs sb vs bb)
 *      P{CO_not, DE, SB_DE, BB_DE_SB}              * E{} * X                       +       (de vs sb vs bb)
 *      P{CO, DE_CO, SB_CO_DE, BB_CO_DE_SB}         * E{} * X                       +       (co vs de vs sb vs bb)
 *
 *
 * BB = P{CO_not, DE_not, SB_not}                   * E{+Sb} * Sb                   +       (empty)
 *      P{CO, DE_CO_not, SB_CO_not, BB_CO_not}      * E{1} * (-Bb)                  +       (only co)
 *      P{CO_not, DE, SB_DE_not, BB_DE_not}         * E{1} * (-Bb)                  +       (only de)
 *      P{CO_not, DE_not, SB, BB_SB_not}            * E{1} * (-Bb)                  +       (only sb)
 *      P{CO, DE_CO, SB_CO_DE_not, BB_CO_DE_not}    * E{1} * (-Bb)                  +       (co vs de)
 *      P{CO, DE_CO_not, SB_CO, BB_CO_SB_not}       * E{1} * (-Bb)                  +       (co vs sb)
 *      P{CO, DE_CO_not, SB_CO_not, BB_CO}          * E{} * X                       +       (co vs bb)
 *      P{CO_not, DE, SB_DE, BB_DE_SB_not}          * E{1} * (-Bb)                  +       (de vs sb)
 *      P{CO_not, DE, SB_DE_not, BB_DE}             * E{} * X                       +       (de vs bb)
 *      P{CO_not, DE_not, SB, BB_SB}                * E{} * X                       +       (sb vs bb)
 *      P{CO, DE_CO, SB_CO_DE, BB_CO_DE_SB_not}     * E{1} * (-Bb)                  +       (co vs de vs sb)
 *      P{CO, DE_CO, SB_CO_DE_not, BB_CO_DE}        * E{} * X                       +       (co vs de vs bb)
 *      P{CO, DE_CO_not, SB_CO, BB_CO_SB}           * E{} * X                       +       (co vs sb vs bb)
 *      P{CO_not, DE, SB_DE, BB_DE_SB}              * E{} * X                       +       (de vs sb vs bb)
 *      P{CO, DE_CO, SB_CO_DE, BB_CO_DE_SB}         * E{} * X                       +       (co vs de vs sb vs bb)
 *
 */


