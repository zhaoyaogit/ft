// Copyright [2020] <Copyright Kevin, kevin.lau.gd@gmail.com>

#ifndef FT_SRC_API_CTP_CTPTRADEAPI_H_
#define FT_SRC_API_CTP_CTPTRADEAPI_H_

#include <ThostFtdcTraderApi.h>
#include <fmt/format.h>

#include <atomic>
#include <cassert>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "Api/Ctp/CtpCommon.h"
#include "Base/DataStruct.h"
#include "Gateway.h"

namespace ft {

class CtpTradeApi : public CThostFtdcTraderSpi {
 public:
  explicit CtpTradeApi(Gateway *gateway);

  ~CtpTradeApi();

  bool login(const LoginParams &params);

  bool logout();

  std::string send_order(const Order *order);

  bool cancel_order(const std::string &order_id);

  bool query_contract(const std::string &ticker);

  bool query_position(const std::string &ticker);

  bool query_account();

  bool query_orders();

  bool query_trades();

  bool query_margin_rate(const std::string &ticker);

  bool query_commision_rate();

  // 当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
  void OnFrontConnected() override;

  // 当客户端与交易后台通信连接断开时，该方法被调用。
  // 当发生这个情况后，API会自动重新连接，客户端可不做处理。
  // @param reason 错误原因
  //         0x1001 网络读失败
  //         0x1002 网络写失败
  //         0x2001 接收心跳超时
  //         0x2002 发送心跳失败
  //         0x2003 收到错误报文
  void OnFrontDisconnected(int reason) override;

  // 心跳超时警告。当长时间未收到报文时，该方法被调用。
  // @param time_lapse 距离上次接收报文的时间
  void OnHeartBeatWarning(int time_lapse) override;

  // 客户端认证响应
  void OnRspAuthenticate(CThostFtdcRspAuthenticateField *auth_rsp,
                         CThostFtdcRspInfoField *rsp_info, int req_id,
                         bool is_last) override;

  // 登录请求响应
  void OnRspUserLogin(CThostFtdcRspUserLoginField *rsp_user_login,
                      CThostFtdcRspInfoField *rsp_info, int req_id,
                      bool is_last) override;

  void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *settlement,
                              CThostFtdcRspInfoField *rsp_info, int req_id,
                              bool is_last) override;

  void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *confirm,
                                  CThostFtdcRspInfoField *rsp_info, int req_id,
                                  bool is_last) override;

  void OnRspUserLogout(CThostFtdcUserLogoutField *user_logout,
                       CThostFtdcRspInfoField *rsp_info, int req_id,
                       bool is_last) override;

  // 拒绝报单
  void OnRspOrderInsert(CThostFtdcInputOrderField *ctp_order,
                        CThostFtdcRspInfoField *rsp_info, int req_id,
                        bool is_last) override;

  void OnRspOrderAction(CThostFtdcInputOrderActionField *action,
                        CThostFtdcRspInfoField *rsp_info, int req_id,
                        bool is_last) override;

  void OnRtnOrder(CThostFtdcOrderField *ctp_order) override;

  // 成交通知
  void OnRtnTrade(CThostFtdcTradeField *trade) override;

  void OnRspQryInstrument(CThostFtdcInstrumentField *instrument,
                          CThostFtdcRspInfoField *rsp_info, int req_id,
                          bool is_last) override;

  void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *position,
                                CThostFtdcRspInfoField *rsp_info, int req_id,
                                bool is_last) override;

  void OnRspQryTradingAccount(CThostFtdcTradingAccountField *trading_account,
                              CThostFtdcRspInfoField *rsp_info, int req_id,
                              bool is_last) override;

  void OnRspQryOrder(CThostFtdcOrderField *order,
                     CThostFtdcRspInfoField *rsp_info, int req_id,
                     bool is_last) override;

  void OnRspQryTrade(CThostFtdcTradeField *trade,
                     CThostFtdcRspInfoField *rsp_info, int req_id,
                     bool is_last) override;

  void OnRspQryInstrumentMarginRate(
      CThostFtdcInstrumentMarginRateField *margin_rate,
      CThostFtdcRspInfoField *rsp_info, int req_id, bool is_last);

 private:
  static std::string get_order_id(const char *instrument, const char *exchange,
                                  const char *order_ref) {
    return fmt::format("{}.{}.{}", instrument, exchange, order_ref);
  }

  int next_req_id() { return next_req_id_++; }

  int next_order_ref() { return next_order_ref_++; }

  void done() { is_done_ = true; }

  void error() { is_error_ = true; }

  void reset_sync() { is_done_ = false; }

  bool wait_sync() {
    while (!is_done_) {
      if (is_error_) return false;
    }

    return true;
  }

 private:
  Gateway *gateway_;
  std::unique_ptr<CThostFtdcTraderApi, CtpApiDeleter> ctp_api_;

  std::string front_addr_;
  std::string broker_id_;
  std::string investor_id_;
  int front_id_ = 0;
  int session_id_ = 0;

  std::atomic<int> next_req_id_ = 0;
  std::atomic<int> next_order_ref_ = 0;

  std::atomic<bool> is_error_ = false;
  std::atomic<bool> is_connected_ = false;
  std::atomic<bool> is_done_ = false;
  std::atomic<bool> is_logon_ = false;

  std::map<std::string, Position> pos_cache_;

  std::mutex query_mutex_;
};

}  // namespace ft

#endif  // FT_SRC_API_CTP_CTPTRADEAPI_H_
