#include <stdio.h>
#include <iostream>
#include "wallet2.h"
#include "wallet/wownero_wallet_full.h"
#include "utils/wownero_utils.h"

using namespace std;

/**
 * Main entry point for scratchpad.
 */
int main(int argc, const char* argv[]) {

  // configure logging
  mlog_configure("log_cpp_scratchpad.txt", true);
  mlog_set_log_level(1);
  //MINFO("logging info!!!");
  //MWARNING("logging a warning!!!");
  //MERROR("logging an error!!!");

  // print header
  MINFO("===== Scratchpad =====");
  for (int i = 0; i < argc; i++) {
    MINFO("Argument" << i << ": " << argv[i]);
  }

  string path = "test_wallet_1";
  string password = "supersecretpassword123";
  string language = "English";
  int network_type = 2;

  // load wallet
  wownero_wallet* wallet = wownero_wallet_full::open_wallet("../../test_wallets/test_wallet_1", "supersecretpassword123", wownero_network_type::TESTNET);
  wallet->set_daemon_connection("http://localhost:28081", "", "");

  // get txs
  vector<shared_ptr<wownero_tx_wallet>> txs = wallet->get_txs();
  MINFO("Wallet has " << txs.size() << " txs");
  for (int i = 0; i < txs.size() && i < 10; i++) MINFO(txs[i]->serialize());
  wownero_utils::free(txs);

  // get transfers
  vector<shared_ptr<wownero_transfer>> transfers = wallet->get_transfers(wownero_transfer_query());
  MINFO("Wallet has " << transfers.size() << " transfers");
  wownero_utils::free(transfers);

  // get outputs
  vector<shared_ptr<wownero_output_wallet>> outputs = wallet->get_outputs(wownero_output_query());
  MINFO("Wallet has " << outputs.size() << " outputs");
  wownero_utils::free(outputs);

  // close wallet and free pointer
  wallet->close(true);
  delete wallet;
}
