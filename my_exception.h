class my_exception : public std::exception {
    public:
    my_exception(const std::string& msg) : msg_(msg) {}
    const char* what(); // override what to return msg_;
    private:
    std::string msg_;
};
