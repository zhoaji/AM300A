#include <time.h>
#include "onmicro_dfu.h"
#include "onmicro_dfu_nvds.h"
#include "utils.h"


typedef struct {
    uint16_t prn;
    uint16_t mtu;
} dfu_env_t;

dfu_env_t dfu_env = {
    .prn = 0,
    .mtu = 517,
};

const dfu_cb_itf_t dfu_cb_itf = { dfu_start_cb, dfu_prog_cb, dfu_end_cb };

void test(char *filename_data, char *filename_bin)
{
    extern void nvds_init(void);
    nvds_init();
    log_debug("PRN:%d, MTU:%d\n", dfu_env.prn, dfu_env.mtu);
    uint8_t *cmd_data, *bin_data;
    uint8_t cmd_prn[] = {0x02, dfu_env.prn >> 8, dfu_env.prn & 0xFF};
    // CMD
    test_write_cmd((uint8_t*)"\x06\x01", 2);
    cmd_prn[2]=dfu_env.prn>>8; cmd_prn[1]=dfu_env.prn&0xFF; test_write_cmd(cmd_prn, sizeof(cmd_prn));
    uint32_t cmd_size = get_file(filename_data, (char**)&cmd_data);
    uint8_t cmd_create[] = {0x01,0x01,(cmd_size>>0)&0xFF,(cmd_size>>8)&0xFF,(cmd_size>>16)&0xFF,(cmd_size>>24)&0xFF};
    test_write_cmd(cmd_create, sizeof(cmd_create));
    test_write_data(cmd_data, cmd_size);
    log_debug("DAT: "); hexdump(cmd_data, cmd_size);
    test_write_cmd((uint8_t*)"\x03", 1);
    test_write_cmd((uint8_t*)"\x04", 1);
    free(cmd_data);
    // DATA
    test_write_cmd((uint8_t*)"\x06\x02", 2);
    cmd_prn[2]=dfu_env.prn>>8; cmd_prn[1]=dfu_env.prn&0xFF; test_write_cmd(cmd_prn, sizeof(cmd_prn));
    uint32_t data_size = get_file(filename_bin, (char**)&bin_data);
    uint8_t data_create[] = {0x01,0x02,(data_size>>0)&0xFF,(data_size>>8)&0xFF,(data_size>>16)&0xFF,(data_size>>24)&0xFF};
    test_write_cmd(data_create, sizeof(data_create));
    uint32_t i, crc = 0;
    for(i=0;i<data_size;i+=dfu_env.mtu-3){
        dfu_response_t rsp = test_write_data(&bin_data[i], data_size-i<dfu_env.mtu-3?data_size-i:dfu_env.mtu-3);
        crc = dfu_crc32(&bin_data[i], data_size-i<dfu_env.mtu-3?data_size-i:dfu_env.mtu-3, &crc);
        if(rsp.length){
            assert(crc == rsp.data.checksum.crc32);
        }
    }
    dfu_response_t rsp = test_write_cmd((uint8_t*)"\x03", 1);
    // Check Result
    log_debug("BIN: 0x%08X, 0x%08X <==> DFU: 0x%08X, 0x%08X\n",
            data_size, crc, rsp.data.checksum.offset, rsp.data.checksum.crc32);
    assert(data_size == rsp.data.checksum.offset);
    assert(crc == rsp.data.checksum.crc32);
    test_write_cmd((uint8_t*)"\x04", 1);
    free(bin_data);
}

int main(int argc, char *argv[])
{
    if(argc <= 2){ log_debug("Usage: Exec x.dat x.bin\n"); return -1; }
    char *filename_data = argv[1];
    char *filename_bin = argv[2];
    int i, j;
    srand(time(0));
    for(i=0;i<100;i++){
        for(j=0;j<=30;j+=10){
            dfu_env.mtu = (rand() % 500) + 23;
            dfu_env.prn = j;
            test(filename_data, filename_bin);
        }
    }
}
