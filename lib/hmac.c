/*
 * hmac.c
 *
 * MontaVista RMCP+ code for doing HMAC, both SHA1 and MD5
 *
 * Author: MontaVista Software, Inc.
 *         Corey Minyard <minyard@mvista.com>
 *         source@mvista.com
 *
 * Copyright 2004 MontaVista Software Inc.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_OPENSSL

#include <errno.h>
#include <string.h>
#include <openssl/hmac.h>
#include <OpenIPMI/ipmi_lan.h>
#include <OpenIPMI/internal/ipmi_malloc.h>

typedef struct hmac_info_s
{
    const EVP_MD *evp_md;
    unsigned char k[20];
} hmac_info_t;

static int
hmac_sha1_init(ipmi_con_t       *ipmi,
	       ipmi_rmcpp_auth_t *ainfo,
	       void             **integ_data)
{
    hmac_info_t   *info;
    unsigned int  klen;

    info = ipmi_mem_alloc(sizeof(*info));
    if (!info)
	return ENOMEM;

    if (ipmi_rmcpp_auth_get_sik_len(ainfo) < 20)
	return EINVAL;

    memcpy(info->k, ipmi_rmcpp_auth_get_sik(ainfo, &klen), 20);

    info->evp_md = EVP_sha1();
    *integ_data = info;
    return 0;
}

static int
hmac_md5_init(ipmi_con_t       *ipmi,
	      ipmi_rmcpp_auth_t *ainfo,
	      void             **integ_data)
{
    hmac_info_t         *info;
    const unsigned char *k;
    unsigned int        klen;

    info = ipmi_mem_alloc(sizeof(*info));
    if (!info)
	return ENOMEM;

    k = ipmi_rmcpp_auth_get_password(ainfo, &klen);
    if (klen < 20)
	return EINVAL;

    memcpy(info->k, k, 20);

    info->evp_md = EVP_md5();
    *integ_data = info;
    return 0;
}

static void
hmac_free(ipmi_con_t *ipmi,
	  void       *integ_data)
{
    hmac_info_t *info = integ_data;

    memset(info->k, 0, sizeof(info->k));
    ipmi_mem_free(integ_data);
}

static int
hmac_add(ipmi_con_t    *ipmi,
	 void          *integ_data,
	 unsigned char *payload,
	 unsigned int  *payload_len,
	 unsigned int  *trailer_len,
	 unsigned int  max_payload_len)
{
    hmac_info_t   *info = integ_data;
    unsigned char *p = payload;
    unsigned int  l = *payload_len;
    unsigned int  ilen;

    if (l+21 > max_payload_len)
	return E2BIG;

    if (l < 4)
	return E2BIG;

    /* We don't authenticate this part of the header. */
    p += 4;
    l -= 4;

    p[l] = 0; /* No padding */
    l++;

    HMAC(info->evp_md, info->k, 20, p, l, p+l, &ilen);

    *payload_len += 1;
    *trailer_len = 20;
    return 0;
}

static int
hmac_check(ipmi_con_t    *ipmi,
	   void          *integ_data,
	   unsigned char *payload,
	   unsigned int  payload_len,
	   unsigned int  total_len)
{
    hmac_info_t   *info = integ_data;
    unsigned char *p = payload;
    unsigned int  l = payload_len;
    unsigned int  ilen;
    unsigned char new_integ[20];

    /* We don't authenticate this part of the header. */
    p += 4;
    l -= 4;

    if ((total_len - payload_len) < 21)
	return EINVAL;

    /* We add 1 to the length because we also check the next header
       field. */
    HMAC(info->evp_md, info->k, 20, p, l+1, new_integ, &ilen);
    if (memcmp(new_integ, p+payload_len+1, 20) != 0)
	return EINVAL;

    return 0;
}

static ipmi_rmcpp_integrity_t hmac_sha1_integ =
{
    .integ_init = hmac_sha1_init,
    .integ_free = hmac_free,
    .integ_add = hmac_add,
    .integ_check = hmac_check
};

static ipmi_rmcpp_integrity_t hmac_md5_integ =
{
    .integ_init = hmac_md5_init,
    .integ_free = hmac_free,
    .integ_add = hmac_add,
    .integ_check = hmac_check
};
#endif /* HAVE_OPENSSL */

int
_ipmi_hmac_init(void)
{
#ifdef HAVE_OPENSSL
    int rv = 0;

    rv = ipmi_rmcpp_register_integrity
	(IPMI_LANP_INTEGRITY_ALGORITHM_HMAC_SHA1_96, &hmac_sha1_integ);
    if (rv)
	return rv;

    rv = ipmi_rmcpp_register_integrity
	(IPMI_LANP_INTEGRITY_ALGORITHM_HMAC_MD5_128, &hmac_md5_integ);
    if (rv)
	return rv;
#endif

    return 0;
}