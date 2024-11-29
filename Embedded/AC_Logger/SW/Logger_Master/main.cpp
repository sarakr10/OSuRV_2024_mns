
#include "glob.hpp"
#include "UART.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>
#include <chrono>

using namespace std;
using namespace std::chrono;


#define BAUDRATE 1000000
#define UART_DEV_PATTERN "/dev/ttyUSB*"

#include "packets.hpp"

#define DEBUG(x) do{ cout <<  #x << " = " << x << endl; }while(0)

#define N_CH 6



static uint64_t msg_rxed = 0;
static uint64_t sync_losted = 0;
static int magics_matched = 0;
void lost_sync() {
	sync_losted++;
	magics_matched = 0;
	cout << "WARN: USB_Logger: sync_lost. " 
		<< sync_losted << " of " << msg_rxed << " lost" << endl;
}

int main() {
	vector<string> usb_devs = glob(UART_DEV_PATTERN);
	cout << "INFO: USB_Logger: usb_devs:" << endl;
	for(const auto usb_dev: usb_devs){
		cout << "\t" << usb_dev << endl;
	}

	vector<shared_ptr<UART>> serials;
	for(auto usb_dev: usb_devs){
		auto s = new UART(
			usb_dev,
			BAUDRATE
		);
		serials.emplace_back(s);
	}
	if(serials.empty()){
		cout << "WARN: USB_Logger: No any USB UART dev plugged in!" << endl;
		return 0;
	}
	
	shared_ptr<UART> serial = serials[serials.size()-1];

#if 1
	ofstream of("dump.tsv", ios::out);
#else
	ostream& of = cout;
#endif
	of << setprecision(10);
	of << "t";
	for(int ch = 0; ch < N_CH; ch++){
		of << '\t' << "ch" << ch;
	}
	
	of << endl;
	
	uint32_t magic_number_buf = 0;
	uint32_t last_id = 0;
	while(true){

		if(magics_matched == 0){
			uint8_t b;
			serial->read(b);
			
			magic_number_buf >>= 8;
			magic_number_buf |= uint32_t(b) << 24;
			if(magic_number_buf == MAGIC){
				magics_matched++;
				serial->skip(
					sizeof(sample_packet)- sizeof(sample_packet::magic)
				);
				msg_rxed++;
			}
		}else{
			//auto t0 = steady_clock::now();
			sample_packet msg;
			serial->read(msg);
			msg_rxed++;
			if(msg.magic != MAGIC){
				lost_sync();
			}else if(magics_matched < 3){
				magics_matched++;
			}else{
				if(msg.id != last_id+1){
					cerr
						<< "Wrong ID!"
						<< "\n\texp = " << (last_id+1)
						<< "\n\tobs = " << msg.id
						<< endl;
				}
				if(msg.err){
					cerr << "Have slipping!" << endl;
				}

				last_id = msg.id;
				double t = msg.id; // TODO calc t
				of << t;
				for(int ch = 0; ch < N_CH; ch++){
					of << '\t' << msg.val_array[ch];
				}
				of << endl;
				t++;
			}
		}
	}
	
	return 0;
}
