/*
 * Copyright (c) 2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_WIFI
#include "dns_resolver.h"
#if PL_CONFIG_USE_PICO_W
  #include "pico/cyw43_arch.h"
#endif
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/ip4_addr.h"
#include "McuLog.h"
#include "McuRTOS.h"

typedef void (*DnsResolver_dns_found_cb_fptr)(const char *hostname, const ip_addr_t *ipaddr, void *arg);

/* Call back with a DNS result */
static void dns_found_cb(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
  DnsResolver_info_t *info = (DnsResolver_info_t*)arg;
  info->dns_response_received = true; /* have received a response */
  if (ipaddr!=NULL) {
    info->resolved_addr = *ipaddr; /* store address */
    info->dns_request_sent = false;
    McuLog_info("'%s' resolved to %s", hostname, ipaddr_ntoa(ipaddr));
  } else {
    McuLog_error("dns request for '%s' failed", hostname);
    /* keep dns_request_sent asserted, so caller knows that it is an error */
  }
}

static DnsResolver_dns_found_cb_fptr dnsFoundCallback = dns_found_cb;

void DnsResolver_SetDnsFoundCallback(DnsResolver_dns_found_cb_fptr callback) {
  dnsFoundCallback = callback;
}

int DnsResolver_ResolveName(const char *name, DnsResolver_info_t *info, int32_t timeoutMs) {
  if (name[0]>='0' && name[0]<='9') { /* IP4 address? */
    u32_t addr_u32 = ipaddr_addr(name); /* convert ASCII (e.g. "192.168.1.1") to a value in network order */
    ip_addr_set_ip4_u32(&info->resolved_addr, addr_u32); /* set the IP address given as u32_t */
  } else {
    info->dns_response_received = false; /* reset flag */
    info->dns_request_sent = true;
    #if PL_CONFIG_USE_PICO_W
    cyw43_arch_lwip_begin();
    #endif
    int err = dns_gethostbyname(name, &info->resolved_addr, dnsFoundCallback, info);
    #if PL_CONFIG_USE_PICO_W
    cyw43_arch_lwip_end();
    #endif

    if (err == ERR_OK) { /* cached result */
      McuLog_info("dns cached result for '%s', OK", name);
      return 0; /* ok */
    } else if (err != ERR_INPROGRESS) { /* ERR_INPROGRESS means expect a callback */
      McuLog_error("dns request failed for '%s", name);
      return -1; /* failed */
    }
    while(timeoutMs>0 && !info->dns_response_received) {
      vTaskDelay(pdMS_TO_TICKS(100)); /* wait for response */
      timeoutMs -= 100;
    }
    if (timeoutMs<=0) {
      McuLog_error("dns request timeout");
      return -2;
    }
    if (info->dns_response_received && info->dns_request_sent) {
      /* sent flag not reset? error */
      McuLog_error("dns response for '%s' received, but not successful", name);
      return -3;
    }
  }
  return 0; /* ok */
}

__attribute__((weak)) void DnsResolver_Init(void) {
}

#endif /* PL_CONFIG_USE_WIFI */