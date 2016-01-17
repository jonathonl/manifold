

#include "user_agent.hpp"

namespace manifold
{
  namespace http
  {
    class user_agent_promise
    {
    public:
      user_agent_promise(request_head&& headers)
        : head_(std::move(headers))
      {
      }

      ~user_agent_promise() {}

      request_head& head()
      {
        return this->head_;
      }

      void set_response(client::response&& resp)
      {
        if (!this->fulfilled_)
        {
          this->fulfilled_ = true;
          if (this->on_response_)
          {
            this->on_response_(std::move(resp));
            this->on_response_ = nullptr;
          }
          else
          {
            this->response_ = std::unique_ptr<client::response>(new client::response(std::move(resp)));
          }
        }
      }

      void on_response(const std::function<void(client::response&&)>& cb)
      {
        if (this->response_)
        {
          auto tmp = std::move(this->response_);
          this->response_ = nullptr;
          cb(std::move(*tmp));
        }
        else
        {
          this->on_response_ = cb;
        }
      }
    private:
      bool fulfilled_ = false;
      request_head head_;
      std::unique_ptr<client::response> response_;
      std::function<void(client::response&&)> on_response_;
    };

    user_agent::request::request(const std::shared_ptr<user_agent_promise>& prom)
      : promise_(prom)
    {
    }

    user_agent::request::request(user_agent::request&& source)
      : promise_(source.promise_)
    {
      source.promise_ = nullptr;
    }

    user_agent::request::~request()
    {
    }

    void user_agent::request::on_response(const std::function<void(client::response&& resp)>& cb)
    {
      this->promise_->on_response(cb);
    }


    user_agent::user_agent(asio::io_service& ioservice)
      : io_service_(ioservice)
    {
    }

    user_agent::~user_agent()
    {
      // TODO: shutdown all clients.
    }

    client& user_agent::get_client(const uri& request_url)
    {
      auto it = this->clients_.find(request_url.socket_address());
      if (it != this->clients_.end())
        return it->second;
      else
      {
        if (request_url.scheme_name() == "https")
        {
          return (this->clients_.emplace(std::make_pair(request_url.socket_address(), client(this->io_service_, request_url.host(), client::ssl_options(), request_url.port()))).first->second);
        }
        else
        {
          return (this->clients_.emplace(std::make_pair(request_url.socket_address(), client(this->io_service_, request_url.host(), request_url.port()))).first->second);
        }
      }
    }


    user_agent::request user_agent::send_request(const std::string& meth, const uri& url, std::stringstream&& entity)
    {
      auto req_prom = std::make_shared<user_agent_promise>(request_head(url.path_with_query(), meth));
      user_agent::request ret(req_prom);
      this->request_promises_.emplace(req_prom);

      client& c = this->get_client(url);

      req_prom->head().authority(url.socket_address());
      auto req_entity = std::make_shared<std::stringstream>(std::move(entity));
      c.on_connect([&c, req_prom, req_entity, this]()
      {
        auto req = std::make_shared<client::request>(c.make_request());
        req->head() = req_prom->head();

        req->on_response([req_prom](client::response&& resp)
        {
          req_prom->set_response(std::move(resp));
        });

        req->on_close([req_prom, this](errc ec)
        {
          // TODO: set error.
          this->request_promises_.erase(req_prom);
          std::size_t num_requests_using_client = 0;
          for (auto it = this->request_promises_.begin(); it != this->request_promises_.end(); ++it)
          {
            if ((*it)->head().authority() == req_prom->head().authority())
              ++num_requests_using_client;
          }

          if (num_requests_using_client == 0)
          {
            auto it = this->clients_.find(req_prom->head().authority());
            if (it != this->clients_.end())
              it->second.close();
            this->clients_.erase(req_prom->head().authority());
          }
        });

        std::array<char, 4096> buf;
        std::size_t bytes_in_buf = req_entity->read(buf.data(), buf.size()).gcount();
        if (!req_entity->good())
        {
          if (bytes_in_buf)
            req->end(buf.data(), bytes_in_buf);
          else
            req->end();
        }
        else
        {
          req->on_drain([req_entity, req]()
          {
            std::array<char, 4096> buf;
            std::size_t bytes_in_buf = req_entity->read(buf.data(), buf.size()).gcount();
            if (bytes_in_buf)
              req->send(buf.data(), bytes_in_buf);

            if (!req_entity->good())
              req->end();
          });
          req->send(buf.data(), bytes_in_buf);
        }
      });


      c.on_close([](errc ec)
      {
        // TODO: set error.
      });

      return ret;
    }
  }
}