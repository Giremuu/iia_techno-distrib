import pytest, json, os
from scanner import check_port, scan_ports


# --- Fixtures pour tests unitaires ---

@pytest.fixture
def localhost():
    return "127.0.0.1"

@pytest.fixture
def json_output(tmp_path):
    return str(tmp_path / "ports.json")


# --- Tests check_port : liste de tuple de valeur ---

@pytest.mark.parametrize("ip, port, exception", [
    (127,          80,    TypeError),   # ip n'est pas un str
    ("127.0.0.1",  "80",  TypeError),   # port n'est pas un int
    ("127.0.0.1",  True,  TypeError),   # bool exclu
    ("127.0.0.1",  0,     ValueError),  # port hors limite
    ("127.0.0.1",  65536, ValueError),  # port hors limite
    ("256.0.0.1",  80,    ValueError),  # IP invalide
    ("not_an_ip",  80,    ValueError),  # IP invalide
])
def test_check_port_invalid_args(ip, port, exception):
    with pytest.raises(exception):
        check_port(ip, port)


@pytest.mark.parametrize("port", [22, 80, 443, 8080])
def test_check_port_returns_valid_dict(localhost, port):
    result = check_port(localhost, port)

    assert isinstance(result, dict)
    assert result["ip"] == localhost
    assert result["port"] == port
    assert result["status"] in ("open", "closed", "error")


# --- Tests scan_ports ---

@pytest.mark.parametrize("ports, exception", [
    ("80,443",  TypeError),   # ports en str au lieu de list
    (80,        TypeError),   # ports en int au lieu de list
    ([],        ValueError),  # liste vide
])
def test_scan_ports_invalid_args(localhost, ports, exception):
    with pytest.raises(exception):
        scan_ports(localhost, ports)


def test_scan_ports_creates_json(localhost, json_output):
    ports = [80, 443]
    scan_ports(localhost, ports, output_file=json_output)

    assert os.path.exists(json_output)
    with open(json_output) as f:
        data = json.load(f)

    assert len(data) == len(ports)
    assert all("status" in entry for entry in data)


def test_scan_ports_returns_list(localhost, json_output):
    result = scan_ports(localhost, [80], output_file=json_output)
    assert isinstance(result, list)
    assert len(result) == 1
