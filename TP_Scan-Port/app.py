from flask import Flask

app = Flask(__name__)

@app.route('/')
def index():
    return "Test-Flask"

app.run(port=5000)