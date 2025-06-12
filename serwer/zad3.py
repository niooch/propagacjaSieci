#!/usr/bin/env python3
import os
import urllib.parse
from http.server import HTTPServer, BaseHTTPRequestHandler

HOST = '0.0.0.0'
PORT = 4321
WWW_ROOT = os.path.join(os.path.dirname(__file__), 'www')

class StaticHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        # ——— Log request ———
        print(f"\n>>> {self.client_address[0]} requested {self.requestline}")
        for name, value in self.headers.items():
            print(f"{name}: {value}")
        print(">>> End of request headers\n")

        # ——— Special paths ———
        if self.path == '/forbidden':
            return self._send_page(403, "Forbidden", "<h1>403 Forbidden</h1><p>You shall not pass!</p>")
        if self.path == '/cause500':
            return self._send_page(500, "Internal Server Error", "<h1>500 Internal Server Error</h1><p>Something went wrong.</p>")

        # ——— Static files ———
        path = urllib.parse.unquote(self.path)
        filename = 'index.html' if path in ('', '/') else path.lstrip('/')
        file_path = os.path.join(WWW_ROOT, filename)

        # security: no escaping outside WWW_ROOT
        if not os.path.commonpath([WWW_ROOT, file_path]) == WWW_ROOT:
            return self._send_page(403, "Forbidden", "<h1>403 Forbidden</h1>")

        if os.path.isfile(file_path):
            # read raw HTML/text
            with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
                raw_html = f.read()
            return self._send_page(200, "OK", raw_html, is_raw=True)
        else:
            return self._send_page(404, "Not Found", "<h1>404 Not Found</h1><p>Page not found.</p>")

    def _send_page(self, code, reason, body_content, is_raw=False):
        """
        Wysyła stronę opakowaną w echo nagłówków.
        Jeśli is_raw=True, body_content traktujemy jako już gotowy HTML.
        """
        # prepare wrapper HTML
        parts = [
            "<!DOCTYPE html>",
            "<html><head><meta charset='utf-8'><title>Headers Echo</title></head><body>",
            f"<h2>Request Line:</h2><pre>{self.requestline}</pre>",
            "<h2>Request Headers:</h2><pre>"
        ]
        for name, value in self.headers.items():
            parts.append(f"{name}: {value}")
        parts.append("</pre>")

        # response headers logging and echo
        resp_headers = {'Connection': 'close'}
        # we'll set Content-Type and Length later

        parts.append("<h2>Response Headers:</h2><pre>")
        # we know we send HTML
        resp_headers['Content-Type'] = 'text/html; charset=utf-8'
        # placeholder; will update after building full_body
        resp_headers['Content-Length'] = '0'
        for k, v in resp_headers.items():
            parts.append(f"{k}: {v}")
        parts.append("</pre><hr>")

        # Body section
        parts.append("<h2>Body:</h2>")
        if is_raw:
            # insert original HTML unescaped
            parts.append(body_content)
        else:
            # plain text content, escape
            escaped = body_content.replace("<", "&lt;").replace(">", "&gt;")
            parts.append(f"<pre>{escaped}</pre>")

        parts.append("</body></html>")

        full_body = "\n".join(parts).encode('utf-8')
        resp_headers['Content-Length'] = str(len(full_body))

        # ——— Log response headers ———
        print(f"<<< Responding {code} {reason}")
        for k, v in resp_headers.items():
            print(f"{k}: {v}")
        print("<<< End of response headers\n")

        # ——— Send to client ———
        self.send_response(code, reason)
        for k, v in resp_headers.items():
            self.send_header(k, v)
        self.end_headers()
        self.wfile.write(full_body)

if __name__ == '__main__':
    print(f"Serving HTTP on {HOST}:{PORT}, root directory: {WWW_ROOT}")
    print(f"Click to open → http://127.0.0.1:{PORT}/\n")
    HTTPServer((HOST, PORT), StaticHandler).serve_forever()

