#include "client.hpp"


static std::unique_ptr<Collector> createCollector(const string& config_file) {
  fs::path path(config_file);
  if (!fs::exists(path) || !fs::is_regular_file(path)) {
    cerr << "Unable to open " << config_file << endl;
    return nullptr;
  }

  pt::ptree config;
  pt::ini_parser::read_ini(config_file, config);

  auto collector = unique_ptr<Collector>(new Collector());

  if (config.find("settings") == config.not_found()) {
    return move(collector);
  } else {
    config = config.get_child("settings");
  }
  
  if (config.find("torrent_save_path") != config.not_found()) {
    collector->set_torrent_save_path(config.get<string>("torrent_save_path"));
  }
  if (config.find("stat_save_path") != config.not_found()) {
    collector->set_stat_save_path(config.get<string>("stat_save_path"));
  }
  if (config.find("magnet_links_path") != config.not_found()) {
    cout << "mag" << config.get<string>("magnet_links_path") << endl;
    collector->set_magnet_links_path(config.get<string>("magnet_links_path"));
  }
  if (config.find("update_interval") != config.not_found()) {
    collector->set_update_interval(config.get<int>("update_interval"));
  }
  if (config.find("session_number") != config.not_found()) {
    collector->set_session_number(config.get<int>("session_number"));
  }
  if (config.find("session_interval") != config.not_found()) {
    collector->set_session_interval(config.get<int>("session_interval"));
  }
  if (config.find("session_start_port") != config.not_found()) {
    collector->set_session_start_port(config.get<int>("session_start_port"));
  }
  if (config.find("save_interval") != config.not_found()) {
    collector->set_save_interval(config.get<int>("save_interval"));
  }

  return move(collector);
}


int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "usage: client config_file.ini" << endl;
    return 1;
  }

  auto collector = createCollector(argv[1]);

  collector->start_session();
  collector->start_work();
  return 0;
}

