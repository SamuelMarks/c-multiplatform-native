# -*- mode: ruby -*-
# vi: set ft=ruby :
#
# Vagrantfile for c-multiplatform-native (cmn)
# Replicates multi-platform builds, tests, and C-driven screenshot generation
# using local Bento boxes built from ../bento (see `vagrant box list`).

Vagrant.configure("2") do |config|
  # ---------------------------------------------------------------------------
  # 1. Linux (Debian 13 ARM64) - GTK3 + Xvfb Native Widget & Screenshot Target
  # ---------------------------------------------------------------------------
  config.vm.define "debian-13", primary: true do |deb|
    deb.vm.box = "bento/debian-13"
    deb.vm.box_check_update = false

    deb.ssh.username = "vagrant"
    deb.ssh.insert_key = false
    deb.ssh.private_key_path = [
      File.expand_path("~/.vagrant.d/insecure_private_keys/vagrant.key.ed25519"),
      File.expand_path("~/.vagrant.d/insecure_private_key")
    ]
    deb.ssh.shell = "/bin/sh"

    deb.vm.provider "qemu" do |qe|
      qe.net_mode = :user
      qe.firmware_format = nil
      bios_candidates = [
        ENV["QEMU_EDK2_CODE"],
        "/opt/homebrew/share/qemu/edk2-aarch64-code.fd",
        "/usr/share/qemu/edk2-aarch64-code.fd",
        "/usr/share/edk2/aarch64/QEMU_EFI.fd"
      ].compact
      bios_path = bios_candidates.find { |path| File.exist?(path) }
      qe.extra_qemu_args = ["-bios", bios_path] if bios_path
      qe.ssh_auto_correct = true
      qe.ssh_port = 50223
      qe.memory = "4G"
      qe.smp = "4"
    end
    deb.vm.usable_port_range = 50200..50299
    deb.vm.allowed_synced_folder_types = [:rsync]

    deb.vm.synced_folder ".", "/vagrant", type: "rsync",
      rsync_exclude: [".git/", "build*/", ".vagrant/", "docs/html/", "docs/latex/", "docs/xml/"]

    deb.vm.provision "shell", inline: <<-SHELL
      set -e
      export DEBIAN_FRONTEND=noninteractive
      apt-get update -qq
      apt-get install -y -qq build-essential cmake git pkg-config libgtk-3-dev libssl-dev libcurl4-openssl-dev xvfb openbox
      cd /vagrant
      rm -rf /tmp/build_linux
      cmake -B /tmp/build_linux -S . -DBUILD_TESTING=ON
      cmake --build /tmp/build_linux --parallel
      mkdir -p /tmp/linux_screenshots
      xvfb-run -a -s "-screen 0 1024x768x24" sh -c "openbox & sleep 1 && /tmp/build_linux/bin/screenshot_gen --output-dir /tmp/linux_screenshots"
    SHELL
  end

  # ---------------------------------------------------------------------------
  # 2. Windows 11 (ARM64) - Win32 / MSVC Native Widget & Screenshot Target
  # ---------------------------------------------------------------------------
  config.vm.define "windows-11", autostart: false do |win|
    win.vm.box = "bento/windows-11"
    win.vm.box_version = "0"
    win.vm.box_check_update = false

    win.vm.guest = :windows
    win.vm.communicator = "ssh"

    win.ssh.username = "vagrant"
    win.ssh.insert_key = false
    win.ssh.private_key_path = File.expand_path("~/.vagrant.d/insecure_private_key")
    win.ssh.shell = "powershell"

    win.vm.provider "qemu" do |qe|
      qe.net_mode = :user
      qe.ssh_auto_correct = true
      qe.ssh_port = 50322
    end
    win.vm.usable_port_range = 50300..50399

    win.vm.network :forwarded_port, id: "rdp", guest: 3389, host: 3389, disabled: true
    win.vm.network :forwarded_port, id: "winrm", guest: 5985, host: 5985, disabled: true

    win.vm.synced_folder ".", "/vagrant", disabled: true
    win.vm.allowed_synced_folder_types = [:rsync]
  end

  # ---------------------------------------------------------------------------
  # 3. RockyLinux 10.2 (ARM64)
  # ---------------------------------------------------------------------------
  config.vm.define "rockylinux-10.2", autostart: false do |rocky|
    rocky.vm.box = "bento/rockylinux-10.2"
    rocky.vm.box_check_update = false
    rocky.vm.synced_folder ".", "/vagrant", type: "rsync"
  end

  # ---------------------------------------------------------------------------
  # 4. Alpine 3.24 (ARM64)
  # ---------------------------------------------------------------------------
  config.vm.define "alpine-3.24", autostart: false do |alpine|
    alpine.vm.box = "bento/alpine-3.24"
    alpine.vm.box_check_update = false
    alpine.vm.synced_folder ".", "/vagrant", type: "rsync"
  end

  # ---------------------------------------------------------------------------
  # 5. FreeBSD 15.1 (ARM64)
  # ---------------------------------------------------------------------------
  config.vm.define "freebsd-15.1", autostart: false do |fbsd|
    fbsd.vm.box = "bento/freebsd-15.1"
    fbsd.vm.box_check_update = false
    fbsd.vm.synced_folder ".", "/vagrant", type: "rsync"
  end

  # ---------------------------------------------------------------------------
  # 6. OmniOS (ARM64)
  # ---------------------------------------------------------------------------
  config.vm.define "omnios", autostart: false do |omnios|
    omnios.vm.box = "bento/omnios"
    omnios.vm.box_check_update = false
    omnios.vm.synced_folder ".", "/vagrant", type: "rsync"
  end
end
