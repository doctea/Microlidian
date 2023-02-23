
struct perf_t {
    unsigned long average_loop_length = 0;
};
perf_t perf_record;

void calc_loop_length(unsigned long start, unsigned long finish) {
    perf_record.average_loop_length += finish-start;
    perf_record.average_loop_length /= 2;
}
