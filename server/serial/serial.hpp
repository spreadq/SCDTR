#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <exception>
#include <thread>
#include <chrono>

class serial_exception: public std::exception {
    virtual const char* what() const throw(){
        return "Unnable to open serial port with those configurations.";
    }
} serial_ex;


class serial_connection {

    private:
        boost::asio::io_service& io_service_; // Has a reference to a io_service
        boost::asio::serial_port sp_;
        boost::asio::streambuf read_buf_;
        bool ready_ = false;

    public:
        serial_connection(boost::asio::io_service& io_service, std::string port_name, int baud_rate)
          : io_service_(io_service),
            sp_(io_service_)
        {
            boost::system::error_code ec;
            sp_.open(port_name, ec);
            if(ec){
                throw serial_ex;
            }
            sp_.set_option(boost::asio::serial_port_base::baud_rate(baud_rate),ec);
            if(ec){
                throw serial_ex;
            }
            wait_for_calibration();
        }

        void read_handler(const boost::system::error_code &ec, size_t nbytes) {
            if (!ec){
                std::cout << &read_buf_ << std::endl;
                std::cout <<"Read " << nbytes << " bytes" << std::endl << std::flush;;
                //boost::asio::async_read(sp_,read_buf_, boost::bind(&serial_connection::read_handler, this,_1,_2));
                boost::asio::async_read_until(sp_,read_buf_,'\n',boost::bind(&serial_connection::read_handler, this,_1,_2));
            }
        }


        // Test function
        void read_lux_to_screen(){
            wait_ready();
            std::cout << "Writing: " << "i" << std::endl;
            boost::asio::write(sp_, boost::asio::buffer("i"));
            std::this_thread::sleep_for (std::chrono::seconds(1));
            //boost::asio::async_read(sp_,read_buf_, boost::bind(&serial_connection::read_handler, this,_1,_2));
            boost::asio::async_read_until(sp_,read_buf_,'\n',boost::bind(&serial_connection::read_handler, this,_1,_2));
        }

        // BLOCKING!
        void wait_ready(){
            while(!ready_){
                std::this_thread::sleep_for (std::chrono::seconds(1));
                //std::cout << "Not done, Buf content: " << &read_buf_ << std::endl;
            }
        }

        /*void check_ready(const boost::system::error_code &ec, size_t nbytes) {
            if(!ec){
                boost::asio::streambuf::const_buffers_type bufs = read_buf_.data();
                std::string line(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + nbytes);
                std::cout << "Read : "<<line << std::endl;
                if(line[0]=='D'){
                    ready_ = true;
                }else{
                    boost::asio::async_read(sp_,read_buf_,boost::bind(&serial_connection::check_ready, this,_1,_2));
                    //  boost::asio::async_read_until(sp_,read_buf_,'\n',boost::bind(&serial_connection::check_ready, this,_1,_2));
                }
            }else{
                delete this;
            }
        }*/

        // Blocks until the system is done calibrating.
        // Reading a byte at a time, not efficient but shouldn't matter.
        void wait_for_calibration(){
            std::cout << "Waiting for the D" << std::endl;
            //boost::asio::async_read_until(sp_,read_buf_,'\n',boost::bind(&serial_connection::check_ready, this,_1,_2));
            //boost::   asio::async_read(sp_,read_buf_,boost::bind(&serial_connection::check_ready, this,_1,_2));
            while(true){
                char c;
                boost::asio::read(sp_, boost::asio::buffer(&c,1));
                if(c=='D'){
                    ready_=true;
                    std::cout << "READY" << std::endl;
                    break;
                }
            }
        }

        // This will also be blocking
        void send_reset(){
            boost::asio::write(sp_, boost::asio::buffer("r"));
            wait_for_calibration();
        }


        //Do answers need to be asynchronous?
        // If we are already sending a query to the Arduino we shouldn't send anything else

    /*    void get_illuminance_at(int i, boost::asio::ip::tcp::socket to){
            wait_ready();
            boost::asio::async_write(sp_, boost::asio::buffer("g l " + i, 6),  boost::bind( &serial_connection::answer_to_socket, this, to, _1));

        }

        void answer_to_socket(boost::asio::ip::tcp::socket to, const boost::system::error_code& error){


        }*/



};