#include "ethernet.h"
#include "utils.h"
#include "driver.h"
#include "arp.h"
#include "ip.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief 处理一个收到的数据包
 *        你需要判断以太网数据帧的协议类型，注意大小端转换
 *        如果是ARP协议数据包，则去掉以太网包头，发送到arp层处理arp_in()
 *        如果是IP协议数据包，则去掉以太网包头，发送到IP层处理ip_in()
 * 
 * @param buf 要处理的数据包
 */
void ethernet_in(buf_t *buf)
{
    // TODO
    net_protocol_t protocol;
    memcpy(&protocol, buf->data+NET_MAC_LEN*2, 2);
    protocol = swap16(protocol);
    buf_remove_header(buf, NET_MAC_LEN*2+2);
    if (protocol == NET_PROTOCOL_ARP) {
        arp_in(buf);
    } else if (protocol == NET_PROTOCOL_IP) {
        ip_in(buf);
    }
}

/**
 * @brief 处理一个要发送的数据包
 *        你需添加以太网包头，填写目的MAC地址、源MAC地址、协议类型
 *        添加完成后将以太网数据帧发送到驱动层
 * 
 * @param buf 要处理的数据包
 * @param mac 目标ip地址
 * @param protocol 上层协议
 */
void ethernet_out(buf_t *buf, const uint8_t *mac, net_protocol_t protocol)
{
    buf_add_header(buf, NET_MAC_LEN*2+2);
    uint16_t tmp[7];
    uint8_t mmac[6] = DRIVER_IF_MAC;
    memcpy(tmp, mac, 6);
    memcpy(tmp+3, mmac, 6);
    // for (int i =0; i<6; ++i) {
    //     tmp[i] = swap16(tmp[i]);
    // }
    tmp[6] = swap16(protocol);
    memcpy(buf->data, tmp, NET_MAC_LEN*2+2);
    printf("%s", buf->data);
    driver_send(buf);
}

/**
 * @brief 初始化以太网协议
 * 
 * @return int 成功为0，失败为-1
 */
int ethernet_init()
{
    buf_init(&rxbuf, ETHERNET_MTU + sizeof(ether_hdr_t));
    return driver_open();
}

/**
 * @brief 一次以太网轮询
 * 
 */
void ethernet_poll()
{
    if (driver_recv(&rxbuf) > 0)
        ethernet_in(&rxbuf);
}
