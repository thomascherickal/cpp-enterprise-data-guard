#!/usr/bin/env python3
"""Lightweight local HTTP server with proper application/wasm MIME type."""

import argparse
import functools
import http.server
import os
import socketserver


class DataGuardHandler(http.server.SimpleHTTPRequestHandler):
    extensions_map = {
        **http.server.SimpleHTTPRequestHandler.extensions_map,
        ".wasm": "application/wasm",
        ".mjs": "application/javascript",
        ".js": "application/javascript",
        ".json": "application/json",
    }

    def end_headers(self):
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Referrer-Policy", "no-referrer")
        super().end_headers()


def main():
    parser = argparse.ArgumentParser(description="Serve Enterprise Data Guard dashboard.")
    parser.add_argument("--dir", default="dist/local/web", help="Directory to serve")
    parser.add_argument("--port", type=int, default=8080, help="Port to listen on")
    parser.add_argument("--host", default="127.0.0.1", help="Host interface to bind")
    args = parser.parse_args()

    directory = os.path.abspath(args.dir)
    handler = functools.partial(DataGuardHandler, directory=directory)
    with socketserver.TCPServer((args.host, args.port), handler) as httpd:
        print(f"Serving Enterprise Data Guard at http://{args.host}:{args.port} (dir={directory})")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down.")


if __name__ == "__main__":
    main()

