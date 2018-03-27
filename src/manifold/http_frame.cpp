
#include "manifold/http_frame.hpp"

//#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include <iostream>

namespace manifold
{
  namespace http
  {
    enum class log_dir { outgoing = 1, incoming };
    void log(const frame_header& f, log_dir direction)
    {
#if 0
      std::cout << (direction == log_dir::outgoing ? "--- OUT ---" : "--- IN ---") << std::endl;
      std::cout << "Stream ID: " << f.stream_id() << std::endl;
      std::cout << "Payload Length: " << f.payload_length() << std::endl;

      std::string str_type;
      switch(f.type())
      {
        case frame_type::data           : str_type = "data" ; break;
        case frame_type::headers        : str_type = "headers" ; break;
        case frame_type::priority       : str_type = "priority" ; break;
        case frame_type::rst_stream     : str_type = "rst_stream" ; break;
        case frame_type::settings       : str_type = "settings" ; break;
        case frame_type::push_promise   : str_type = "push_promise" ; break;
        case frame_type::ping           : str_type = "ping" ; break;
        case frame_type::goaway         : str_type = "goaway" ; break;
        case frame_type::window_update  : str_type = "window_update" ; break;
        case frame_type::continuation   : str_type = "continuation" ; break;
        case frame_type::invalid_type   : str_type = "invalid" ; break;
      }
      std::cout << "Frame Type: " << str_type << std::endl;
      std::cout << std::endl;
#endif
    }
    //****************************************************************//
    // frame header
    //----------------------------------------------------------------//
    frame_header::frame_header()
    //: metadata_{{'\0','\0','\0','\0','\0','\0','\0','\0','\0'}}
    {
      this->metadata_.fill('\0');
      frame_type t = frame_type::invalid_type;
      memcpy(this->metadata_.data() + 3, &t, 1);
    };
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    frame_header::frame_header(std::uint32_t payload_length, frame_type t, std::uint8_t flags, std::uint32_t stream_id)
    {
      std::uint32_t payload_length_24bit_nbo = htonl(payload_length << 8);
      std::uint32_t stream_id_nbo = htonl(stream_id);
      memcpy(this->metadata_.data(), &payload_length_24bit_nbo, 3);
      memcpy(this->metadata_.data() + 3, &t, 1);
      memcpy(this->metadata_.data() + 4, &flags, 1);
      memcpy(this->metadata_.data() + 5, &stream_id_nbo, 4); // assuming first bit is zero.
    };
    //----------------------------------------------------------------//


    //----------------------------------------------------------------//
    frame_header::frame_header(frame_header&& source)
    {
      this->metadata_.fill('\0');
      frame_type t = frame_type::invalid_type;
      memcpy(this->metadata_.data() + 3, &t, 1);

      operator=(std::move(source));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    frame_header& frame_header::operator=(frame_header&& source)
    {
      if (&source != this)
      {
        this->metadata_ = std::move(source.metadata_);
      }
      return (*this);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    frame_header::~frame_header()
    {
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t frame_header::payload_length() const
    {
      std::uint32_t ret = 0;
      memcpy(&ret, this->metadata_.data(), 3);
      return ((ntohl(ret) >> 8) & 0x00FFFFFF);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    frame_type frame_header::type() const
    {
      std::uint8_t tmp;
      memcpy(&tmp, this->metadata_.data() + 3, 1);

      switch (tmp)
      {
      case (int)frame_type::data           : return frame_type::data         ;
      case (int)frame_type::headers        : return frame_type::headers      ;
      case (int)frame_type::priority       : return frame_type::priority     ;
      case (int)frame_type::rst_stream     : return frame_type::rst_stream   ;
      case (int)frame_type::settings       : return frame_type::settings     ;
      case (int)frame_type::push_promise   : return frame_type::push_promise ;
      case (int)frame_type::ping           : return frame_type::ping         ;
      case (int)frame_type::goaway         : return frame_type::goaway       ;
      case (int)frame_type::window_update  : return frame_type::window_update;
      case (int)frame_type::continuation   : return frame_type::continuation ;
      default: return frame_type::invalid_type ;
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint8_t frame_header::flags() const
    {
      std::uint8_t ret;
      memcpy(&ret, this->metadata_.data() + 4, 1);
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t frame_header::stream_id() const
    {
      std::uint32_t ret;
      memcpy(&ret, this->metadata_.data() + 5, 4);
      return (ntohl(ret) & 0x7FFFFFFF);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void frame_header::recv(socket& sock, frame_header& destination, asio::yield_context yctx)
    {
      std::size_t bytes_read = sock.recv(destination.metadata_.data(), 9, yctx);

      if (yctx.ec_ && *yctx.ec_)
      {

      }
      else
      {
        log(destination, log_dir::incoming);
        if (destination.type() == frame_type::invalid_type)
        {
          frame_type t = frame_type::invalid_type;
          memcpy(destination.metadata_.data() + 3, &t, 1);
          // Will be ignored.
        }
      }

    }
    //template void frame::recv_frame<asio::ip::tcp::socket>(asio::ip::tcp::socket& sock, frame& source, const std::function<void(const std::error_code& ec)>& cb);
    //template void frame::recv_frame<asio::ssl::stream<asio::ip::tcp::socket>>(asio::ssl::stream<asio::ip::tcp::socket>& sock, frame& source, const std::function<void(const std::error_code& ec)>& cb);
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void frame_header::send(socket& sock, const frame_header& source, asio::yield_context yctx)
    {
      log(source, log_dir::outgoing);
      std::size_t bytes_transfered = sock.send(source.metadata_.data(), source.metadata_.size(), yctx);
    }

    //template void frame::send_frame<asio::ip::tcp::socket>(asio::ip::tcp::socket& sock, const frame& source, const std::function<void(const std::error_code& ec)>& cb);
    //template void frame::send_frame<asio::ssl::stream<asio::ip::tcp::socket>>(asio::ssl::stream<asio::ip::tcp::socket>& sock, const frame& source, const std::function<void(const std::error_code& ec)>& cb);
    //----------------------------------------------------------------//
    //****************************************************************//

    //****************************************************************//
    // frame_payload
    //----------------------------------------------------------------//
    std::uint32_t frame_payload::serialized_length() const
    {
      return (std::uint32_t)this->buf_.size();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void frame_payload::recv(socket& sock, frame_payload& destination, asio::yield_context yctx)
    {
      frame_header::recv(sock, destination, yctx);

      if (destination.type() != frame_type::invalid_type)
      {
        destination.buf_.resize(destination.payload_length());
        sock.recv(destination.buf_.data(), destination.payload_length(), yctx);
      }
    }
    //template void frame_payload::recv_frame_payload<asio::ip::tcp::socket>(asio::ip::tcp::socket& sock, frame_payload& destination, std::uint32_t payload_size, std::uint8_t flags, const std::function<void(const std::error_code& ec)>& cb);
    //template void frame_payload::recv_frame_payload<asio::ssl::stream<asio::ip::tcp::socket>>(asio::ssl::stream<asio::ip::tcp::socket>& sock, frame_payload& destination, std::uint32_t payload_size, std::uint8_t flags, const std::function<void(const std::error_code& ec)>& cb);
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    void frame_payload::send(socket& sock, const frame_payload& source, asio::yield_context yctx)
    {
      frame_header::send(sock, source, yctx);
      if (!yctx.ec_ || !yctx.ec_->value())
        sock.send(source.buf_.data(), source.buf_.size(), yctx);
    }
    //template void frame_payload::send_frame_payload<asio::ip::tcp::socket>(asio::ip::tcp::socket& sock, const frame_payload& source, const std::function<void(const std::error_code& ec)>& cb);
    //template void frame_payload::send_frame_payload<asio::ssl::stream<asio::ip::tcp::socket>>(asio::ssl::stream<asio::ip::tcp::socket>& sock, const frame_payload& source, const std::function<void(const std::error_code& ec)>& cb);
    //----------------------------------------------------------------//
    //****************************************************************//



    //****************************************************************//
    // data_frame
    //----------------------------------------------------------------//
    data_frame::data_frame(std::uint32_t stream_id, const char*const data, std::uint32_t datasz, bool end_stream, const char*const padding, std::uint8_t paddingsz) :
      frame_payload(
        padding && paddingsz ? (datasz + 1 + paddingsz) : datasz,
        frame_type::data,
        (end_stream ? frame_flag::end_stream : (std::uint8_t)0x0) | (padding && paddingsz ? frame_flag::padded : (std::uint8_t)0x0),
        stream_id)
    {
      if (this->flags() & frame_flag::padded)
      {
        this->buf_.resize(datasz + 1 + paddingsz);
        this->buf_[0] = paddingsz;
        memcpy(this->buf_.data() + 1, data, datasz);
        memcpy(this->buf_.data() + 1 + datasz, padding, paddingsz);
      }
      else
      {
        this->buf_.resize(datasz);
        memcpy(this->buf_.data(), data, datasz);
      }


      memcpy(this->buf_.data(), data, datasz);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    data_frame data_frame::split(std::uint32_t num_bytes)
    {
      std::uint32_t this_data_length = this->data_length();
      if (num_bytes > this_data_length)
        num_bytes = this_data_length;


      data_frame ret(stream_id(), this->data(), num_bytes);
      data_frame::operator=(data_frame(stream_id(), data() + num_bytes, data_length() - num_bytes, flags() & frame_flag::end_stream, padding(), pad_length()));
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const char*const data_frame::data() const
    {
      if (this->flags() & frame_flag::padded)
        return this->buf_.data() + 1;
      else
        return this->buf_.data();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t data_frame::data_length() const
    {
      return (std::uint32_t)(this->buf_.size() - (this->pad_length() + (this->flags() & frame_flag::padded ? 1 : 0)));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const char*const data_frame::padding() const
    {
      if (this->flags() & frame_flag::padded)
        return this->buf_.data() + 1 + this->data_length();
      else
        return nullptr;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint8_t data_frame::pad_length() const
    {
      std::uint8_t ret = 0;
      if (this->flags() & frame_flag::padded)
        memcpy(&ret, this->buf_.data(), 1);
      return ret;
    }
    //----------------------------------------------------------------//
    //****************************************************************//

    //****************************************************************//
    // headers_frame
    //----------------------------------------------------------------//
    headers_frame::headers_frame(std::uint32_t stream_id, const char*const header_block, std::uint32_t header_block_sz, bool end_headers, bool end_stream, const char*const padding, std::uint8_t paddingsz) :
      frame_payload(
        (paddingsz ? 1 : 0) + header_block_sz + ((paddingsz ? 1 : 0) ? paddingsz : 0),
        frame_type::headers,
        (end_stream ? frame_flag::end_stream : (std::uint8_t)0x0)
          | (end_headers ? frame_flag::end_headers : (std::uint8_t)0x0)
          | (padding && paddingsz ? frame_flag::padded : (std::uint8_t)0x0),
        stream_id)
    {
      this->buf_.resize(this->bytes_needed_for_pad_length() + header_block_sz + (this->bytes_needed_for_pad_length() ? paddingsz : 0));

      memcpy(this->buf_.data() + this->bytes_needed_for_pad_length(), header_block, header_block_sz);
      if (this->flags() & frame_flag::padded)
      {
        this->buf_[0] = paddingsz;
        memcpy(this->buf_.data() + this->bytes_needed_for_pad_length() + header_block_sz, padding, paddingsz);
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    headers_frame::headers_frame(std::uint32_t stream_id, const char*const header_block, std::uint32_t header_block_sz, bool end_headers, bool end_stream, priority_options priority_ops, const char*const padding, std::uint8_t paddingsz) :
      frame_payload(
        (paddingsz ? 1 : 0) + 5 + header_block_sz + ((paddingsz ? 1 : 0) ? paddingsz : 0),
        frame_type::headers,
        (end_stream ? frame_flag::end_stream : (std::uint8_t)0x0)
          | (end_headers ? frame_flag::end_headers : (std::uint8_t)0x0)
          | (frame_flag::priority)
          | (padding && paddingsz ? frame_flag::padded : (std::uint8_t)0x0),
        stream_id)
    {
      this->buf_.resize(this->bytes_needed_for_pad_length() + 5 + header_block_sz + (this->bytes_needed_for_pad_length() ? paddingsz : 0));
      if (this->flags() & frame_flag::padded)
      {
        this->buf_[0] = paddingsz;
        memcpy(this->buf_.data() + this->bytes_needed_for_pad_length() + 5 + header_block_sz, padding, paddingsz);
      }
      std::uint32_t tmp_nbo = htonl(priority_ops.exclusive ? (0x80000000 ^ priority_ops.stream_dependency_id) : (0x7FFFFFFF & priority_ops.stream_dependency_id));
      memcpy(this->buf_.data() + this->bytes_needed_for_pad_length(), &tmp_nbo, 4);
      memcpy(this->buf_.data() + this->bytes_needed_for_pad_length() + 4, &priority_ops.weight, 1);
      memcpy(this->buf_.data() + this->bytes_needed_for_pad_length() + 5, header_block, header_block_sz);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint8_t headers_frame::bytes_needed_for_pad_length() const
    {
      return (std::uint8_t)(this->flags() & frame_flag::padded ? 1 : 0);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint8_t headers_frame::bytes_needed_for_dependency_id_and_exclusive_flag() const
    {
      return (std::uint8_t)(this->flags() & frame_flag::priority ? 4 : 0);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint8_t headers_frame::bytes_needed_for_weight() const
    {
      return (std::uint8_t)(this->flags() & frame_flag::priority ? 1 : 0);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const char*const headers_frame::header_block_fragment() const
    {
      return (this->buf_.data() + this->bytes_needed_for_pad_length() + this->bytes_needed_for_dependency_id_and_exclusive_flag() + this->bytes_needed_for_weight());
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t headers_frame::header_block_fragment_length() const
    {
      return (std::uint32_t)(this->buf_.size() - (this->pad_length() + this->bytes_needed_for_pad_length() + this->bytes_needed_for_dependency_id_and_exclusive_flag() + this->bytes_needed_for_weight()));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const char*const headers_frame::padding() const
    {
      if (this->flags() & frame_flag::padded)
        return this->buf_.data() + 1 + this->bytes_needed_for_dependency_id_and_exclusive_flag() + this->bytes_needed_for_weight() + this->header_block_fragment_length();
      else
        return nullptr;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint8_t headers_frame::pad_length() const
    {
      std::uint8_t ret = 0;
      if (this->flags() & frame_flag::padded)
        memcpy(&ret, this->buf_.data(), 1);
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint8_t headers_frame::weight() const
    {
      std::uint8_t ret = 0;
      if (this->flags() & frame_flag::priority)
        memcpy(&ret, this->buf_.data() + this->bytes_needed_for_pad_length() + 4, 1);
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t headers_frame::stream_dependency_id() const
    {
      std::uint32_t ret = 0;
      if (this->flags() & frame_flag::priority)
        memcpy(&ret, this->buf_.data() + this->bytes_needed_for_pad_length(), 4);
      return (0x7FFFFFFF & ntohl(ret));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool headers_frame::exclusive_stream_dependency() const
    {
      std::uint8_t tmp = 0;
      if (this->flags() & frame_flag::priority)
        memcpy(&tmp, this->buf_.data() + this->bytes_needed_for_pad_length(), 1);

      return (0x80 & tmp) != 0;
    }
    //----------------------------------------------------------------//
    //****************************************************************//

    //****************************************************************//
    // priority_frame
    //----------------------------------------------------------------//
    priority_frame::priority_frame(std::uint32_t stream_id, priority_options options) : frame_payload(5, frame_type::priority, 0x0, stream_id)
    {
      this->buf_.resize(5);
      std::uint32_t tmp = (options.exclusive ? (0x80000000 ^ options.stream_dependency_id) : (0x7FFFFFFF & options.stream_dependency_id));
      std::uint32_t tmp_nbo = htonl(tmp);
      memcpy(this->buf_.data(), &tmp_nbo, 4);
      memcpy(this->buf_.data() + 4, &(options.weight), 1);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint8_t priority_frame::weight() const
    {
      std::uint8_t ret;
      memcpy(&ret, this->buf_.data() + 4, 1);
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t priority_frame::stream_dependency_id() const
    {
      std::uint32_t ret;
      memcpy(&ret, this->buf_.data(), 4);
      return (0x7FFFFFFF & ntohl(ret));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    bool priority_frame::exclusive_stream_dependency() const
    {
      std::uint8_t tmp;
      memcpy(&tmp, this->buf_.data(), 1);

      return (0x80 & tmp) != 0;
    }
    //----------------------------------------------------------------//
    //****************************************************************//

    //****************************************************************//
    // rst_stream_frame
    //----------------------------------------------------------------//
    rst_stream_frame::rst_stream_frame(std::uint32_t stream_id, http::v2_errc error_code) :
      frame_payload(4, frame_type::rst_stream, 0x0, stream_id)
    {
      this->buf_.resize(4);
      std::uint32_t error_code_nbo = htonl((std::uint32_t)error_code);
      memcpy(this->buf_.data(), &error_code_nbo, 4);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t rst_stream_frame::error_code() const
    {
      std::uint32_t tmp;
      memcpy(&tmp, this->buf_.data(), 4);
      return ntohl(tmp);
    }
    //----------------------------------------------------------------//
    //****************************************************************//

    //****************************************************************//
    // settings_frame
    //----------------------------------------------------------------//
    settings_frame::settings_frame(std::list<std::pair<std::uint16_t,std::uint32_t>>::const_iterator beg, std::list<std::pair<std::uint16_t,std::uint32_t>>::const_iterator end) :
      frame_payload(6 * std::distance(beg, end), frame_type::settings, 0x0, 0)
    {
      this->buf_.resize(6 * std::distance(beg, end));
      std::size_t pos = 0;
      for (auto it = beg; it != end; ++it)
      {
        std::uint16_t key_nbo(htons(it->first));
        std::uint32_t value_nbo(htonl(it->second));
        memcpy(&this->buf_[pos], &key_nbo,  2);
        memcpy(&this->buf_[pos + 2], &value_nbo,  4);
        pos = pos + 6;
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::list<std::pair<std::uint16_t,std::uint32_t>> settings_frame::settings() const
    {
      std::list<std::pair<std::uint16_t,std::uint32_t>> ret;

      std::size_t bytesToParse = this->buf_.size();
      std::size_t pos = 0;
      while (bytesToParse >= 6)
      {
        std::uint16_t key;
        std::uint32_t value;
        memcpy(&key, &this->buf_[pos], 2);
        memcpy(&value, &this->buf_[pos + 2], 4);
        ret.push_back(std::pair<std::uint16_t,std::uint32_t>(ntohs(key), ntohl(value)));
        pos = pos + 6;
        bytesToParse = bytesToParse - 6;
      }

      return ret;
    }
    //----------------------------------------------------------------//
    //****************************************************************//

    //****************************************************************//
    // push_promise_frame
    //----------------------------------------------------------------//
    push_promise_frame::push_promise_frame(std::uint32_t stream_id, const char*const header_block, std::uint32_t header_block_sz, std::uint32_t promise_stream_id, bool end_headers, const char*const padding, std::uint8_t paddingsz) :
      frame_payload(
        (paddingsz ? 1 : 0) + 4 + header_block_sz + paddingsz,
        frame_type::push_promise,
        (std::uint8_t)(end_headers ? frame_flag::end_headers : 0x0) | (std::uint8_t)(padding && paddingsz ? frame_flag::padded : 0),
        stream_id)
    {
      if (this->flags() & frame_flag::padded)
      {
        this->buf_.resize(5 + header_block_sz + paddingsz);
        this->buf_[0] = paddingsz;
        std::uint32_t promise_stream_id_nbo = htonl(0x7FFFFFFF & promise_stream_id);
        memcpy(this->buf_.data() + 1, &promise_stream_id_nbo, 4);
        memcpy(this->buf_.data() + 5, header_block, header_block_sz);
        memcpy(this->buf_.data() + 5 + header_block_sz, padding, paddingsz);
      }
      else
      {
        this->buf_.resize(4 + header_block_sz);
        std::uint32_t promise_stream_id_nbo = htonl(0x7FFFFFFF & promise_stream_id);
        memcpy(this->buf_.data(), &promise_stream_id_nbo, 4);
        memcpy(this->buf_.data() + 4, header_block, header_block_sz);
      }
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const char*const push_promise_frame::header_block_fragment() const
    {
      if (this->flags() & frame_flag::padded)
        return (this->buf_.data() + 5);
      else
        return (this->buf_.data() + 4);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t push_promise_frame::header_block_fragment_length() const
    {
      if (this->flags() & frame_flag::padded)
        return (std::uint32_t)(this->buf_.size() - (this->pad_length() + 5));
      else
        return (std::uint32_t)(this->buf_.size() - 4);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const char*const push_promise_frame::padding() const
    {
      if (this->flags() & frame_flag::padded)
        return (this->buf_.data() + 5 + this->header_block_fragment_length());
      else
        return nullptr;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint8_t push_promise_frame::pad_length() const
    {
      std::uint8_t ret = 0;
      if (this->flags() & frame_flag::padded)
        memcpy(&ret, this->buf_.data(), 1);
      return ret;
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t push_promise_frame::promised_stream_id() const
    {
      std::uint32_t ret;
      memcpy(&ret, this->buf_.data() + (this->flags() & frame_flag::padded ? 1 : 0), 4);
      return (0x7FFFFFFF & ntohl(ret));
    }
    //----------------------------------------------------------------//
    //****************************************************************//

    //****************************************************************//
    // ping_frame
    //----------------------------------------------------------------//
    ping_frame::ping_frame(std::uint64_t ping_data, bool ack) :
      frame_payload(8, frame_type::ping, std::uint8_t(ack ? 0x1 : 0x0), 0)
    {
      this->buf_.resize(8);
      std::uint64_t ping_data_nbo = htonll(ping_data);
      memcpy(this->buf_.data(), &ping_data_nbo, 8);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint64_t ping_frame::data() const
    {
      std::uint64_t ret;
      memcpy(&ret, this->buf_.data(), 8);
      return ntohll(ret);
    }
    //----------------------------------------------------------------//
    //****************************************************************//

    //****************************************************************//
    // goaway_frame
    //----------------------------------------------------------------//
    goaway_frame::goaway_frame(std::uint32_t last_stream_id, http::v2_errc error_code, const char*const addl_error_data, std::uint32_t addl_error_data_sz) :
      frame_payload(8 + addl_error_data_sz, frame_type::goaway, 0x0, 0)
    {
      this->buf_.resize(8 + addl_error_data_sz);
      std::uint32_t tmp_nbo = htonl(0x7FFFFFFF & last_stream_id);
      std::uint32_t error_code_nbo = htonl((std::uint32_t)error_code);
      memcpy(this->buf_.data(), &tmp_nbo, 4);
      memcpy(this->buf_.data() + 4, &error_code_nbo, 4);
      memcpy(this->buf_.data() + 8, addl_error_data, addl_error_data_sz);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t goaway_frame::last_stream_id() const
    {
      std::uint32_t ret;
      memcpy(&ret, this->buf_.data(), 4);
      return (0x7FFFFFFF & ntohl(ret));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    http::v2_errc goaway_frame::error_code() const
    {
      std::uint32_t tmp;
      memcpy(&tmp, this->buf_.data() + 4, 4);
      return int_to_v2_errc(ntohl(tmp));
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const char*const goaway_frame::additional_debug_data() const
    {
      return (this->buf_.data() + 8);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t goaway_frame::additional_debug_data_length() const
    {
      return (std::uint32_t)(this->buf_.size() - 8);
    }
    //----------------------------------------------------------------//
    //****************************************************************//

    //****************************************************************//
    // window_update_frame
    //----------------------------------------------------------------//
    window_update_frame::window_update_frame(std::uint32_t stream_id, std::uint32_t window_size_increment) :
      frame_payload(4, frame_type::window_update, 0x0, stream_id)
    {
      this->buf_.resize(4);
      std::uint32_t tmp_nbo = htonl(0x7FFFFFFF & window_size_increment);
      memcpy(this->buf_.data(), &tmp_nbo, 4);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::int32_t window_update_frame::window_size_increment() const
    {
      std::uint32_t ret;
      memcpy(&ret, this->buf_.data(), 4);
      return std::int32_t(0x7FFFFFFF & ntohl(ret));
    }
    //----------------------------------------------------------------//
    //****************************************************************//

    //****************************************************************//
    // continuation_frame
    //----------------------------------------------------------------//
    continuation_frame::continuation_frame(std::uint32_t stream_id, const char*const header_data, std::uint32_t header_data_sz, bool end_headers) :
      frame_payload(header_data_sz, frame_type::continuation, end_headers ? frame_flag::end_headers : (std::uint8_t)0, stream_id)
    {
      this->buf_.resize(header_data_sz);
      memcpy(this->buf_.data(), header_data, header_data_sz);
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    const char*const continuation_frame::header_block_fragment() const
    {
      return this->buf_.data();
    }
    //----------------------------------------------------------------//

    //----------------------------------------------------------------//
    std::uint32_t continuation_frame::header_block_fragment_length() const
    {
      return (std::uint32_t)this->buf_.size();
    }
    //----------------------------------------------------------------//
    //****************************************************************//
  }
}