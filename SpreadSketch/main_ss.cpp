#include "spreadsketch.hpp"
#include "inputadaptor.hpp"
#include <utility>
#include "datatypes.hpp"
#include "util.h"
#include <fstream>
#include <iomanip>

#include "topK.hpp"

int main(int argc, char* argv[]) {
	if (argc < 3) {
		cout << "Usage Error:\n";
		cout << "argv[1]: int rows\n";
		cout << "argv[2]: int columns\n";
		cout << "argv[3]: int top-K\n";
		cout << "argv[4]: string Trace\n";
		return 0;
	}

    // Configure parameters
    // buffer size
    unsigned long long buf_size = 500000000;
    // Superspreader threshold
    double thresh = 0.001;
    // SpreadSketch parameters
    int lgn = 32;
    int cmdepth = atoi(argv[1]);
    //int cmdepth = 4;
    //int cmwidth = 14096;
    int cmwidth = atoi(argv[2]);
    //int cmwidth = 4;
    int b = 79;
    int c = 3;
    int memory = 438;
    int total_mem = cmdepth*cmwidth*(memory+lgn+8)/1024/8;

    int k_val = atoi(argv[3]);

    std::string file = std::string(argv[4]);

    // Result array
    double precision = 0, recall = 0, error = 0, throughput = 0, dtime = 0;
    double avgpre = 0, avgrec = 0, avgerr = 0, avgthr = 0, avgdet = 0;
    int epoch = 0;

        InputAdaptor* adaptor =  new InputAdaptor(file, buf_size, 1);
        std::cout << "[Dataset]: " << file << std::endl;
        std::cout << "[Message] Finish read data." << std::endl;
        // Get the ground truth
        mymap ground;
	orderedMapTopK< uint64_t, uint64_t> topK(k_val);
        edgeset sum;
        edge_tp t;
        adaptor->Reset();
        while(adaptor->GetNext(&t) == 1) {
            sum.insert(t);
            //ground[t.src_ip].insert(t.dst_ip);
	    
	    //std::cout << (((t.src_ip) & 0xFF000000) >> 24) << ".";
	    //std::cout << (((t.src_ip) & 0xFF0000) >> 16) << ".";
	    //std::cout << (((t.src_ip) & 0xFF00) >> 8) << ".";
	    //std::cout << (((t.src_ip) & 0xFF)) << std::endl;
	    
	    uint32_t key = t.dst_ip;
	    //uint64_t val = (uint64_t) t.src_ip | ((uint64_t) t.src_port) << 32 | ((uint64_t) t.dst_port) << 48;
	    uint32_t val = t.src_ip; 

	    ground[key].insert(val);
	    topK.update(key, ground[key].size());
        }
        std::cout << "[Message] Finish Insert hash table" << std::endl;

        // Calculate the threshold value
        double threshold = thresh * sum.size();
        std::cout << "[Message] Total distinct paris: " << sum.size() << "  threshold=" << threshold<< std::endl;

        // Create sketch
        DetectorSS *sketch = new DetectorSS(cmdepth, cmwidth, lgn, b, c, memory);

        // Update sketch
        double t1=0, t2=0;
        double datasize = adaptor->GetDataSize();
        t1 = now_us();
        adaptor->Reset();
        while(adaptor->GetNext(&t) == 1) {
            //sketch->Update(t.src_ip, t.dst_ip, 1);
	    uint32_t key = t.dst_ip;
	    //uint64_t val = (uint64_t) t.src_ip | ((uint64_t) t.src_port) << 32 | ((uint64_t) t.dst_port) << 48;
	    uint32_t val = t.src_ip; 
            sketch->Update(key, val, 1);
        }
        t2 = now_us();
        throughput = datasize/(double)(t2-t1)*1000000;

        // Query the result
        t1=0, t2=0;
        myvector results;
        results.clear();
        t1 = now_us();
        sketch->Query(threshold, results);
        t2 = now_us();
        dtime = (double)(t2-t1)/1000000;

        // Calculate accuracy
	//std::cout << "--------------------------" << std::endl;
	//std::cout << "<val> \t <key> \t\t|\t truth" << std::endl;
	//std::cout << "==========VVV=============" << std::endl;
        //int tp = 0, cnt = 0;;
        //for (auto it = ground.begin(); it != ground.end(); it++) {
        //    if (it->second.size() > threshold || 1) {
        //        cnt++;
        //        int truth = (int) it->second.size();
        //        for (auto res = results.begin(); res != results.end(); res++) {
        //            if (res->first == it->first) {
        //                error += abs((int)res->second - truth)*1.0/truth;

	//		/*
	//		std::cout << res->second << "\t"; 
	//		std::cout << (((res->first) & 0xFF000000) >> 24) << ".";
	//		std::cout << (((res->first) & 0xFF0000) >> 16) << ".";
	//		std::cout << (((res->first) & 0xFF00) >> 8) << ".";
	//		std::cout << (((res->first) & 0xFF)) << "";
	//		std::cout << "      \t|\t" << truth << std::endl;
	//		*/
        //                tp++;
        //            }
        //        }
        //    }
        //} 
	//std::cout << "==========V=============" << std::endl;
	//std::cout << std::endl;
        //std::cout << "[Message] Total " << cnt << " superspreaders, detect " << tp << std::endl;
        //std::cout << "[Message] Total " << results.size() << " superspreaders, detect " << tp << std::endl;
        //precision = tp*1.0/results.size();
        //recall = tp*1.0/cnt;
        //error = error/tp;

        //avgpre += precision; avgrec += recall; avgerr += error; avgthr += throughput; avgdet += dtime;
        //delete sketch;
        //delete adaptor;

        // Calculate accuracy
	std::cout << "--------------------------" << std::endl;
	std::cout << "TOP-K" << std::endl;
	std::cout << "--------------------------" << std::endl;
	std::cout << "<val> \t <key> \t\t|\t truth" << std::endl;
	std::cout << "==========VVV=============" << std::endl;
        int tp = 0, cnt = 0;;
	vector<pair<uint64_t, uint64_t>> topK_HH = topK.items();
        for (auto it = topK_HH.begin(); it != topK_HH.end(); it++) {
            if (it->second > threshold || 1) {
                cnt++;
                int truth = (int) it->second;
                for (auto res = results.begin(); res != results.end(); res++) {
                    if (res->first == it->first) {
                        error += abs((int)res->second - truth)*1.0/truth;

			std::cout << res->second << "\t"; 
			std::cout << (((res->first) & 0xFF000000) >> 24) << ".";
			std::cout << (((res->first) & 0xFF0000) >> 16) << ".";
			std::cout << (((res->first) & 0xFF00) >> 8) << ".";
			std::cout << (((res->first) & 0xFF)) << "";
			std::cout << "      \t|\t" << truth << std::endl;
                        tp++;
                    }
                }
            }
        } 
	std::cout << "==========V=============" << std::endl;
	std::cout << std::endl;
        std::cout << "[Message] Truth " << cnt << " superspreaders, detect " << tp << std::endl;
        std::cout << "[Message] " << results.size() << " keys saved, tp-key detect " << tp << std::endl;
        precision = tp*1.0/k_val;
        recall = tp*1.0/cnt;
        error = error/tp;

        avgpre += precision; avgrec += recall; avgerr += error; avgthr += throughput; avgdet += dtime;
        delete sketch;
        delete adaptor;

        //Output to standard output
        std::cout << std::setfill(' ');
        std::cout << std::setw(20) << std::left << "Memory(KB)" << std::setw(20) << std::left << "Precision"
            << std::setw(20) << std::left << "Recall" << std::setw(20)
            << std::left << "Error" << std::setw(20) << std::left << "Throughput" << std::setw(20)
            << std::left << "DTime" << std::endl;
        std::cout << std::setw(20) << std::left << total_mem << std::setw(20) << std::left << precision << std::setw(20)
            << std::left << recall << std::setw(20) << std::left << error << std::setw(20) << std::left << throughput << std::setw(20)
            << std::left << dtime << std::endl;

}

