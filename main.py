import requests
import argparse

def fetch_package_info(package_name):
    base_url = "https://archlinux.org/packages/search/json/?name="
    response = requests.get(base_url + package_name)
    if response.status_code == 200:
        package_info = response.json()
        if package_info['results']:
            pkg = package_info['results'][0]
            return {
                'name': pkg['pkgname'],
                'description': pkg['pkgdesc'],
                'repository': pkg['repo'],
                'version': pkg['pkgver'],
                'url': pkg['url'],
                'license': ', '.join(pkg['licenses']),
                'dependencies': ', '.join(pkg['depends'])
            }
        else:
            return None
    else:
        return None

def main():
    parser = argparse.ArgumentParser(
        description="Fetch information about a package in the Arch Linux repositories."
    )
    parser.add_argument('package_name', type=str, help='Name of the package to fetch information for')
    args = parser.parse_args()

    package_info = fetch_package_info(args.package_name)

    if package_info:
        print(f"Package Name: {package_info['name']}")
        print(f"Description: {package_info['description']}")
        print(f"Repository: {package_info['repository']}")
        print(f"Version: {package_info['version']}")
        print(f"URL: {package_info['url']}")
        print(f"License: {package_info['license']}")
        print(f"Dependencies: {package_info['dependencies']}")
    else:
        print(f"Package '{args.package_name}' not found in official repositories.")

if __name__ == "__main__":
    main()
