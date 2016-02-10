#include <cstdlib>
#include <cstdio>
#include <string>
#include <ostream>
#include <libtorrent/session.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/alert.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/create_torrent.hpp>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


using namespace std;
namespace lt = libtorrent;
namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

class Collector {
  protected:
    int update_interval;
    int session_number;
    int session_interval;
    int session_start_port;
    int save_interval;

    string torrent_save_path;
    string stat_save_path;
    string magnet_links_path;

    // libtorrent session settings
    int upload_rate_limit;
    int download_rate_limit;
    int active_downloads;
    int alert_queue_size;
    int dht_announce_interval;
    int torrent_upload_limit;
    int torrent_download_limit;
    
    vector<lt::session*> sessions;
    map<lt::sha1_hash, int> info_hash2count;
    map<lt::sha1_hash, string> info_hash2torrent_files;

    map<string, int> trackers;
    vector<lt::add_torrent_params> torrents;

    void _handle_alerts(lt::session *s, deque<lt::alert*>& alerts);
    void _add_torrent(lt::session* s, const lt::sha1_hash& info_hash);

    void _summarize_statistics();
    void _print_statistics();

  public:
    Collector() :
      update_interval(20), session_number(20), session_interval(60), session_start_port(32900),
      save_interval(60),
      torrent_save_path("./torrents"),
      stat_save_path("./result.csv"),
      magnet_links_path(""),
      upload_rate_limit(200000),
      download_rate_limit(200000),
      active_downloads(30),
      alert_queue_size(4000),
      dht_announce_interval(40),
      torrent_upload_limit(200000),
      torrent_download_limit(200000)
      {}

    void start_session();
    void start_work();

    void load_torrents();

    inline void set_session_number(int v)               { session_number = v; }
    inline void set_update_interval(int v)              { update_interval = v; }
    inline void set_session_interval(int v)             { session_interval = v; }
    inline void set_session_start_port(int v)           { session_start_port = v; }
    inline void set_save_interval(int v)                { save_interval = v; }

    inline void set_torrent_save_path(const string& p)  { torrent_save_path = p; }
    inline void set_stat_save_path(const string& p)     { stat_save_path = p; }
    inline void set_magnet_links_path(const string& p)  { magnet_links_path = p; }

    inline void set_upload_rate_limit(int v)            { upload_rate_limit = v; }
    inline void set_download_rate_limit(int v)          { download_rate_limit = v; }
    inline void set_alert_queue_size(int v)             { alert_queue_size = v; }
    inline void set_dht_announce_interval(int v)        { dht_announce_interval = v; }
    inline void set_torrent_upload_limit(int v)         { torrent_upload_limit = v; }
    inline void set_torrent_download_limit(int v)       { torrent_download_limit = v; }
};

