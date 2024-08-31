# #!/usr/bin/env python3

# import os

# def print_cgi_environment():
#     """Prints CGI environment variables."""
#     print("Content-Type: text/html\n")  # HTTP header for HTML content
#     print("<html><head><title>CGI Environment Variables</title></head><body>")
#     print("<h1>CGI Environment Variables</h1>")
#     print("<table border='1'>")
#     print("<tr><th>Variable</th><th>Value</th></tr>")
    
#     for key, value in sorted(os.environ.items()):
#         print(f"<tr><td>{key}</td><td>{value}</td></tr>")
    
#     print("</table>")
#     print("</body></html>")

# if __name__ == "__main__":
#     print_cgi_environment()


#!/usr/bin/env python3

import os
import cgi
import cgitb
import sys

# Enable error logging to the browser
cgitb.enable()

# Set max file upload size (in bytes)
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10 MB

# Directory where uploaded files will be stored
UPLOAD_DIR = "/tmp/uploads"

# Ensure the upload directory exists
os.makedirs(UPLOAD_DIR, exist_ok=True)

def main():
    print("Content-Type: text/html\n")

    # Get the HTTP method and content length
    method = os.environ.get("REQUEST_METHOD", "GET")
    content_length = os.environ.get("CONTENT_LENGTH")

    if method != "POST":
        print("<h1>405 Method Not Allowed</h1>")
        print("<p>Use POST method to upload files.</p>")
        return

    # Check if the content length is valid
    if content_length:
        try:
            content_length = int(content_length)
            if content_length > MAX_FILE_SIZE:
                print("<h1>413 Request Entity Too Large</h1>")
                print(f"<p>File size exceeds {MAX_FILE_SIZE} bytes.</p>")
                return
        except ValueError:
            print("<h1>400 Bad Request</h1>")
            print("<p>Invalid Content-Length.</p>")
            return

    # Parse form data
    form = cgi.FieldStorage()

    # Check if a file was uploaded
    if 'file' not in form:
        print("<h1>400 Bad Request</h1>")
        print("<p>No file was uploaded.</p>")
        return

    # Get the uploaded file
    file_item = form['file']

    # Check if the file was actually uploaded
    if not file_item.filename:
        print("<h1>400 Bad Request</h1>")
        print("<p>No file was uploaded.</p>")
        return

    # Construct a safe file path
file_name = os.path.basename(file_item.filename)
    file_path = os.path.join(UPLOAD_DIR, file_name)

    # Save the file to the specified directory
    try:
        with open(file_path, 'wb') as file_out:
            file_out.write(file_item.file.read())
        print("<h1>File Uploaded Successfully</h1>")
        print(f"<p>File saved as: {file_path}</p>")
    except IOError as e:
        print("<h1>500 Internal Server Error</h1>")
        print(f"<p>Error saving file: {str(e)}</p>")

if __name__ == "__main__":
    main()
