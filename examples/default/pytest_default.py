from pytest_embedded import Dut
from pytest_embedded_idf.utils import idf_parametrize


def expect_selection(
    dut: Dut,
    menu: str,
    index: int,
    options: tuple[str, ...],
    *,
    timeout: float = 8,
) -> None:
    """Verify one complete menu block printed by the display callback."""
    dut.expect_exact("========== MENU ==========", timeout=timeout)
    dut.expect_exact(f"Menu: {menu}", timeout=timeout)
    dut.expect_exact("--------------------------", timeout=timeout)
    for option_index in range(len(options) - 1, -1, -1):
        option = options[option_index]
        marker = ">" if option_index == index else " "
        dut.expect_exact(f"{marker} {option}", timeout=timeout)
    dut.expect_exact("==========================", timeout=timeout)


@idf_parametrize("target", ["esp32"], indirect=["target"])
def test_default_menu_flow(dut: Dut) -> None:
    """Exercise the simulated navigation sequence on a real ESP target."""
    root_options = ("submenu1", "submenu2", "submenu3")
    submenu_options = ("funcA", "funcB", "funcC")

    dut.expect_exact("Start menu", timeout=10)
    expect_selection(dut, "root", 0, root_options)
    dut.expect_exact("Root Title: root", timeout=5)

    dut.expect_exact("Command received: UP", timeout=15)
    dut.expect_exact("Command UP", timeout=5)
    expect_selection(dut, "root", 1, root_options)

    dut.expect_exact("Command received: SELECT", timeout=8)
    dut.expect_exact("Open Submenu", timeout=5)
    expect_selection(dut, "submenu2", 0, submenu_options)

    dut.expect_exact("Command received: DOWN", timeout=8)
    dut.expect_exact("Command DOWN", timeout=5)
    expect_selection(dut, "submenu2", 2, submenu_options)

    dut.expect_exact("Command received: SELECT", timeout=8)
    dut.expect_exact("Execute Function: funcC", timeout=5)
    dut.expect_exact("I am dumb", timeout=5)
    expect_selection(dut, "submenu2", 2, submenu_options)

    dut.expect_exact("Command received: BACK", timeout=8)
    expect_selection(dut, "submenu2", 2, submenu_options)

    dut.expect_exact("Command received: BACK", timeout=8)
    dut.expect_exact("Command BACK", timeout=5)
    expect_selection(dut, "root", 0, root_options)

    dut.expect_exact("Finalizada", timeout=15)
