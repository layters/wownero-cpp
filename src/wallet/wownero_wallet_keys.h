/**
 * Copyright (c) woodser
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Parts of this file are originally copyright (c) 2014-2019, The Monero Project
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * All rights reserved.
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers
 */

#pragma once

#include "wownero_wallet.h"
#include "cryptonote_basic/account.h"

using namespace wownero;

/**
 * Public library interface.
 */
namespace wownero {

  /**
   * Implements a Wownero wallet to provide basic key management.
   */
  class wownero_wallet_keys : public wownero_wallet {

  public:

    // --------------------------- STATIC WALLET UTILS --------------------------

    /**
     * Create a new wallet with a randomly generated seed.
     *
     * @param config is the wallet configuration (network type and language)
     */
    static wownero_wallet_keys* create_wallet_random(const wownero_wallet_config& config);

    /**
     * Create a wallet from an existing mnemonic phrase or seed.
     *
     * @param config is the wallet configuration (network type, seed, seed offset, isMultisig)
     */
    static wownero_wallet_keys* create_wallet_from_seed(const wownero_wallet_config& config);

    /**
     * Create a wallet from an address, view key, and spend key.
     * 
     * @param config is the wallet configuration (network type, address, view key, spend key, language)
     */
    static wownero_wallet_keys* create_wallet_from_keys(const wownero_wallet_config& config);

    /**
     * Get a list of available languages for the wallet's seed.
     *
     * @return the available languages for the wallet's seed
     */
    static std::vector<std::string> get_seed_languages();

    // ----------------------------- WALLET METHODS -----------------------------

    /**
     * Destruct the wallet.
     */
    ~wownero_wallet_keys();

    /**
     * Supported wallet methods.
     */
    bool is_view_only() const override { return m_is_view_only; }
    wownero_version get_version() const override;
    wownero_network_type get_network_type() const override { return m_network_type; }
    std::string get_seed() const override { return m_seed; }
    std::string get_seed_language() const override { return m_language; }
    std::string get_private_view_key() const override { return m_prv_view_key; }
    std::string get_private_spend_key() const override { return m_prv_spend_key; }
    std::string get_public_view_key() const override { return m_pub_view_key; }
    std::string get_public_spend_key() const override { return m_pub_spend_key; }
    std::string get_primary_address() const override { return m_primary_address; }
    std::string get_address(const uint32_t account_idx, const uint32_t subaddress_idx) const override;
    wownero_integrated_address get_integrated_address(const std::string& standard_address = "", const std::string& payment_id = "") const override;
    wownero_integrated_address decode_integrated_address(const std::string& integrated_address) const override;
    wownero_account get_account(const uint32_t account_idx, bool include_subaddresses) const override;
    std::vector<wownero_subaddress> get_subaddresses(const uint32_t account_idx, const std::vector<uint32_t>& subaddress_indices) const override;
    std::string sign_message(const std::string& msg, wownero_message_signature_type signature_type, uint32_t account_idx = 0, uint32_t subaddress_idx = 0) const override;
    wownero_message_signature_result verify_message(const std::string& msg, const std::string& address, const std::string& signature) const override;
    void close(bool save = false) override;

    // --------------------------------- PRIVATE --------------------------------

  private:
    bool m_is_view_only;
    wownero_network_type m_network_type;
    cryptonote::account_base m_account;
    std::string m_seed;
    std::string m_language;
    std::string m_pub_view_key;
    std::string m_prv_view_key;
    std::string m_pub_spend_key;
    std::string m_prv_spend_key;
    std::string m_primary_address;

    void init_common();
  };
}
