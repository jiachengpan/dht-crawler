#include "collector.hpp"

void Collector::start_session() {
  load_torrents();
  for (int port = session_start_port; port < session_start_port+session_number; ++port) {
    lt::session* s = new lt::session();

    s->set_alert_mask(lt::alert::all_categories);
    s->listen_on(make_pair(port, port));

    for (auto it = trackers.begin(); it != trackers.end(); ++it) {
      s->add_dht_router(make_pair(it->first, it->second));
    }
    s->add_dht_router(make_pair("router.bittorrent.com", 6881));
    s->add_dht_router(make_pair("router.utorrent.com", 6881));
    s->add_dht_router(make_pair("router.bitcomet.com", 6881));
    s->add_dht_router(make_pair("dht.transmissionbt.com", 6881));

    lt::session_settings st(s->settings());
    // configure settings.
    
    for (auto t : torrents) {
      s->add_torrent(t);
    }
    sessions.push_back(s);
  }
}

void Collector::start_work() {
  lt::ptime end_time = lt::time_now() + lt::seconds(session_interval);
  lt::ptime save_time = lt::time_now() + lt::seconds(save_interval);

  while (1) {
    if (lt::is_negative(end_time - lt::time_now())) break;
    if (lt::is_negative(save_time - lt::time_now())) {
      _summarize_statistics();
      save_time = lt::time_now() + lt::seconds(save_interval);
    }

    _print_statistics();
    for (lt::session* s : sessions) {
      s->post_torrent_updates();

      deque<lt::alert*> alerts;
      s->pop_alerts(&alerts);
      _handle_alerts(s, alerts);
    }
    sleep(update_interval);
  }

  _summarize_statistics();
}

void Collector::load_torrents() {
  const boost::regex expression(".*://(.*):([0-9]+)");

  if (magnet_links_path.empty()) return;
  cout << "loading magnet links from: " << magnet_links_path << endl;

  fs::path mpath(magnet_links_path);

  if (fs::exists(mpath) && fs::is_regular_file(mpath)) {
    ifstream ifs(mpath.native().c_str());
    string line;


    while (ifs >> line) {
      lt::add_torrent_params param;
      lt::error_code ec;
      lt::parse_magnet_uri(line, param, ec);

      if (ec) {
        cout << ec.message() << endl;
        continue;
      }

      param.flags = lt::add_torrent_params::flag_share_mode;
      param.save_path = torrent_save_path;

      torrents.push_back(param);
      for (string& t : param.trackers) {
        boost::smatch matches;
        if (boost::regex_match(t, matches, expression)) {
          trackers[matches[1]] = stoi(matches[2]);
        }
      }
    }
  }
}


void Collector::_handle_alerts(lt::session *s, deque<lt::alert*>& alerts) {
  for (lt::alert* alt : alerts) {
    switch (alt->type()) {
      case lt::add_torrent_alert::alert_type:
      {
        auto _alt = dynamic_cast<lt::add_torrent_alert*>(alt);
        _alt->handle.set_upload_limit(20000);
        _alt->handle.set_download_limit(20000);
        break;
      }

      case lt::dht_get_peers_alert::alert_type:
      {
        auto _alt = dynamic_cast<lt::dht_get_peers_alert*>(alt);
        lt::sha1_hash hash = _alt->info_hash;
        if (info_hash2count.find(hash) != info_hash2count.end()) {
          info_hash2count[hash] += 1;
        } else {
          info_hash2count[hash] = 1;
        }

        break;
      }

      case lt::dht_announce_alert::alert_type:
      {
        auto _alt = dynamic_cast<lt::dht_announce_alert*>(alt);
        lt::sha1_hash hash = _alt->info_hash;
        if (info_hash2count.find(hash) != info_hash2count.end()) {
          info_hash2count[hash] += 1;
        } else {
          info_hash2count[hash] = 1;
        }
        cout << "announce " << hash << endl;
        _add_torrent(s, hash);

        break;
      }
      
      case lt::metadata_received_alert::alert_type:
      {
        auto _alt = dynamic_cast<lt::metadata_received_alert*>(alt);

        auto info = _alt->handle.torrent_file();

        if (info_hash2torrent_files.find(info->info_hash()) != info_hash2torrent_files.end()) {
          break;
        }
        lt::create_torrent ct(*info);
        lt::entry entry = ct.generate();
        string buffer;
        lt::bencode(back_inserter(buffer), entry);
        info_hash2torrent_files[info->info_hash()] = buffer;
        cout << "saved: " << lt::to_hex(info->info_hash().to_string()) << endl;

        s->remove_torrent(_alt->handle);
        break;
      }
    }
  }
}

void Collector::_add_torrent(lt::session* s, const lt::sha1_hash& info_hash) {
  if (info_hash2torrent_files.find(info_hash) != info_hash2torrent_files.end()) {
    cout << "skip" << endl;
    return;
  }

  lt::add_torrent_params param;
  param.info_hash = info_hash;
  param.flags = lt::add_torrent_params::flag_share_mode;
  param.save_path = torrent_save_path;

  s->add_torrent(param);
}

void Collector::_summarize_statistics() {
  boost::property_tree::ptree result;

  for (auto iter = info_hash2torrent_files.begin(); iter != info_hash2torrent_files.end(); ++iter) {
    pt::ptree child_torrent;

    child_torrent.put("hotness", info_hash2count[iter->first]);
    child_torrent.put("torrent", iter->second);

    result.push_back(make_pair(lt::to_hex(iter->first.to_string()), child_torrent));
  }

  boost::property_tree::json_parser::write_json(stat_save_path, result);
  cout << "result saved at " << stat_save_path << endl;
}

void Collector::_print_statistics() {
  int idx = 0;
  for (lt::session* s : sessions) {
    cout << "session " << idx++ << "\t";
    cout << "dht_running: " << s->is_dht_running() << "\t";
    lt::session_status stat = s->status();
    cout << "num_peers: " << stat.num_peers << "\t";
    cout << "dht_nodes: " << stat.dht_nodes << endl;
  }
}

